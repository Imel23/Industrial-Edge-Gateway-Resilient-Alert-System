// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * industrial_dht11.c - DHT11 IIO Driver for BeagleBone Black
 *
 * This driver implements the DHT11 temperature and humidity sensor
 * using the Linux Industrial I/O (IIO) subsystem. It bit-bangs the
 * proprietary 1-wire protocol over a single GPIO pin configured as
 * open-drain (or push-pull with direction switching).
 *
 * IIO channels exposed:
 *   - in_temp_raw       : Raw temperature value (millidegrees Celsius)
 *   - in_humidityrelative_raw : Raw relative humidity (millipercent)
 *
 * Protocol overview (DHT11 1-wire, 40-bit frame):
 *   1. Host pulls data line LOW for >= 18 ms  (start signal)
 *   2. Host releases line (goes HIGH via pull-up)
 *   3. Sensor responds with ~80 µs LOW, then ~80 µs HIGH
 *   4. Sensor sends 40 data bits, each preceded by a ~50 µs LOW:
 *        - "0" bit: ~26-28 µs HIGH pulse
 *        - "1" bit: ~70 µs HIGH pulse
 *   5. Data format: [8-bit humidity integer] [8-bit humidity decimal]
 *                   [8-bit temperature integer] [8-bit temperature decimal]
 *                   [8-bit checksum]
 *   6. Checksum = (hum_int + hum_dec + temp_int + temp_dec) & 0xFF
 *
 * Timing constraints:
 *   - Minimum 1 second between readings (sensor limitation)
 *   - Bit discrimination threshold: ~40 µs HIGH = '0', >40 µs = '1'
 *   - All timing-critical sections run with IRQs disabled
 *
 * Error handling:
 *   - Checksum mismatch returns -EIO
 *   - Communication timeout returns -ETIMEDOUT
 *   - Readings faster than 1s apart return cached data or -EAGAIN
 *
 * Device Tree binding:
 *   dht11_sensor {
 *       compatible = "industrial,dht11-iio";
 *       pinctrl-names = "default";
 *       pinctrl-0 = <&project_dht11_pin>;
 *       gpios = <&gpio1 13 0>;
 *   };
 *
 * Copyright (C) 2026  Imel23
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/version.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define DRIVER_NAME		"industrial_dht11"

/*
 * DHT11 communication timing constants (in microseconds)
 *
 * These values come from the DHT11 datasheet and include small
 * tolerance margins for real-world signal jitter.
 */
#define DHT11_START_LOW_US	20000	/* Host pulls LOW for >=18 ms */
#define DHT11_START_HIGH_US	40	/* Host releases; wait for sensor */
#define DHT11_RESPONSE_US	80	/* Sensor LOW + HIGH response ~80+80 µs */
#define DHT11_BIT_THRESHOLD_US	40	/* <40 µs HIGH = '0', >=40 µs = '1' */
#define DHT11_TIMEOUT_US	100	/* Max wait for any single edge */

/*
 * Minimum interval between successive reads (in jiffies).
 * The DHT11 datasheet mandates >= 1 second between samples.
 * We use 2 seconds for safety margin.
 */
#define DHT11_MIN_INTERVAL	(2 * HZ)

/* Number of data bits transmitted by the DHT11 per frame */
#define DHT11_DATA_BITS		40

/* Number of data bytes (40 bits / 8) */
#define DHT11_DATA_BYTES	5

/**
 * struct dht11_data - Private driver data for a DHT11 sensor instance.
 *
 * @gpio:        GPIO descriptor for the single data line.
 * @lock:        Mutex serialising access to the sensor (only one
 *               transaction at a time is allowed).
 * @dev:         Pointer to the parent platform device (for dev_*() logging).
 * @temperature: Last successfully decoded temperature (millidegrees C).
 * @humidity:    Last successfully decoded humidity (millipercent RH).
 * @last_read:   jiffies timestamp of the last successful sensor read,
 *               used to enforce the minimum polling interval.
 */
struct dht11_data {
	struct gpio_desc	*gpio;
	struct mutex		lock;
	struct device		*dev;

	/* Cached sensor values (millidegrees C / millipercent RH) */
	int			temperature;
	int			humidity;

	/* Rate-limiting: jiffies of last successful read */
	unsigned long		last_read;
};

/* ------------------------------------------------------------------ */
/*  Low-level 1-wire bit-bang helpers                                  */
/* ------------------------------------------------------------------ */

/**
 * dht11_wait_for_level() - Spin-wait until GPIO reaches @level or timeout.
 * @gpio:  GPIO descriptor to poll.
 * @level: Expected logic level (0 or 1).
 * @max_us: Maximum time to wait in microseconds.
 *
 * Busy-loops reading the GPIO pin until either the desired level is
 * observed or @max_us microseconds have elapsed. Must be called with
 * interrupts disabled for accurate timing.
 *
 * Return: Duration waited in microseconds, or -ETIMEDOUT on timeout.
 */
static int dht11_wait_for_level(struct gpio_desc *gpio, int level, int max_us)
{
	int elapsed = 0;

	while (gpiod_get_value(gpio) != level) {
		if (elapsed >= max_us)
			return -ETIMEDOUT;
		udelay(1);
		elapsed++;
	}

	return elapsed;
}

/**
 * dht11_read_raw_data() - Execute the DHT11 1-wire protocol and capture bits.
 * @data: Driver private data containing GPIO descriptor and device pointer.
 * @buf:  Output buffer for 5 bytes of raw sensor data.
 *
 * Performs the complete DHT11 communication handshake:
 *   1. Send start signal (pull LOW ~20 ms, release)
 *   2. Wait for sensor response (two pulses: LOW then HIGH, ~80 µs each)
 *   3. Read 40 data bits by measuring each HIGH pulse duration
 *
 * All timing-critical sections run with local IRQs disabled to prevent
 * scheduling jitter from corrupting the bit-bang timing. This is
 * acceptable because the total critical section is approximately
 * 4-5 ms (40 bits × ~120 µs/bit + response).
 *
 * Return: 0 on success, -ETIMEDOUT if any edge transition times out.
 */
static int dht11_read_raw_data(struct dht11_data *data, u8 buf[DHT11_DATA_BYTES])
{
	unsigned long flags;
	int i, ret;
	int bit_times[DHT11_DATA_BITS];

	/*
	 * ---- START SIGNAL ----
	 * Pull the data line LOW for at least 18 ms to signal the sensor.
	 * This can be done with IRQs enabled since we only need ~20 ms delay.
	 */
	gpiod_direction_output(data->gpio, 0);
	msleep(20);

	/*
	 * Release the line and switch to input mode.
	 * The external pull-up resistor will bring the line HIGH.
	 */
	gpiod_direction_input(data->gpio);
	udelay(DHT11_START_HIGH_US);

	/*
	 * ---- CRITICAL TIMING SECTION ----
	 * Disable interrupts to get accurate µs-level timing for
	 * the sensor response and data bit decoding.
	 */
	local_irq_save(flags);

	/* Wait for sensor response: LOW pulse (~80 µs) */
	ret = dht11_wait_for_level(data->gpio, 0, DHT11_TIMEOUT_US);
	if (ret < 0) {
		dev_dbg(data->dev, "Timeout waiting for sensor response LOW\n");
		goto out_irq;
	}

	ret = dht11_wait_for_level(data->gpio, 1, DHT11_RESPONSE_US);
	if (ret < 0) {
		dev_dbg(data->dev, "Timeout during sensor response LOW phase\n");
		goto out_irq;
	}

	/* Wait for sensor response: HIGH pulse (~80 µs) */
	ret = dht11_wait_for_level(data->gpio, 0, DHT11_RESPONSE_US);
	if (ret < 0) {
		dev_dbg(data->dev, "Timeout during sensor response HIGH phase\n");
		goto out_irq;
	}

	/*
	 * ---- READ 40 DATA BITS ----
	 * Each bit starts with a ~50 µs LOW pulse (start of bit),
	 * followed by a HIGH pulse whose duration encodes the bit value:
	 *   - 26-28 µs HIGH → '0'
	 *   - ~70 µs HIGH   → '1'
	 */
	for (i = 0; i < DHT11_DATA_BITS; i++) {
		/* Wait for start-of-bit LOW to end */
		ret = dht11_wait_for_level(data->gpio, 1, DHT11_RESPONSE_US);
		if (ret < 0) {
			dev_dbg(data->dev,
				"Timeout at bit %d start (waiting for HIGH)\n", i);
			goto out_irq;
		}

		/* Measure duration of HIGH pulse (determines bit value) */
		ret = dht11_wait_for_level(data->gpio, 0, DHT11_RESPONSE_US);
		if (ret < 0) {
			dev_dbg(data->dev,
				"Timeout at bit %d data (waiting for LOW)\n", i);
			goto out_irq;
		}

		bit_times[i] = ret;
	}

	local_irq_restore(flags);

	/*
	 * ---- DECODE BITS ----
	 * Convert measured HIGH-pulse durations to bit values.
	 * Threshold: >= 40 µs → '1', < 40 µs → '0'
	 */
	memset(buf, 0, DHT11_DATA_BYTES);
	for (i = 0; i < DHT11_DATA_BITS; i++) {
		buf[i / 8] <<= 1;
		if (bit_times[i] >= DHT11_BIT_THRESHOLD_US)
			buf[i / 8] |= 1;
	}

	return 0;

out_irq:
	local_irq_restore(flags);
	return -ETIMEDOUT;
}

/* ------------------------------------------------------------------ */
/*  Data validation and decoding                                       */
/* ------------------------------------------------------------------ */

/**
 * dht11_validate_checksum() - Verify the integrity of a DHT11 data frame.
 * @buf: 5-byte raw data from the sensor.
 *
 * The DHT11 checksum is the least significant 8 bits of the sum of
 * the first four data bytes. This catches single-bit errors and most
 * multi-bit corruption that can occur due to timing inaccuracies or
 * electrical noise.
 *
 * Return: 0 if checksum is valid, -EIO otherwise.
 */
static int dht11_validate_checksum(const u8 buf[DHT11_DATA_BYTES])
{
	u8 expected = (buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF;

	if (buf[4] != expected)
		return -EIO;

	return 0;
}

/**
 * dht11_decode() - Decode raw sensor bytes into temperature and humidity.
 * @data: Driver private data (updated with decoded values).
 * @buf:  5-byte validated raw data.
 *
 * DHT11 data format (5 bytes):
 *   Byte 0: Humidity integer part (0-100)
 *   Byte 1: Humidity decimal part (0 for DHT11, nonzero for some variants)
 *   Byte 2: Temperature integer part (bit 7 = sign, bits 6:0 = magnitude)
 *   Byte 3: Temperature decimal part (tenths of degree, some variants)
 *   Byte 4: Checksum (already validated before calling this function)
 *
 * Values are stored in millidegrees C and millipercent RH for
 * compatibility with the IIO subsystem scale conventions.
 *
 * This function handles both classic DHT11 (decimal = 0) and
 * variant DHT11 sensors that report fractional values.
 */
static void dht11_decode(struct dht11_data *data, const u8 buf[DHT11_DATA_BYTES])
{
	int temp_int = buf[2];
	int temp_dec = buf[3];
	int hum_int  = buf[0];
	int hum_dec  = buf[1];

	/*
	 * Temperature decoding:
	 * - Integer part: bits 6:0 of byte 2
	 * - Sign: bit 7 of byte 2 (1 = negative)
	 * - Decimal part: byte 3 (tenths of degree for some variants)
	 * - Result in millidegrees: int * 1000 + dec * 100
	 */
	data->temperature = (temp_int & 0x7F) * 1000 + temp_dec * 100;
	if (temp_int & 0x80)
		data->temperature = -data->temperature;

	/*
	 * Humidity decoding:
	 * - Integer part: byte 0
	 * - Decimal part: byte 1 (usually 0 for DHT11)
	 * - Result in millipercent: int * 1000 + dec * 100
	 */
	data->humidity = hum_int * 1000 + hum_dec * 100;
}

/* ------------------------------------------------------------------ */
/*  Sensor read with rate limiting, retries and error handling         */
/* ------------------------------------------------------------------ */

/**
 * dht11_do_read() - Perform a sensor reading with rate limiting and retries.
 * @data: Driver private data.
 *
 * Enforces the DHT11's minimum 2-second interval between reads. If called
 * too soon, returns -EAGAIN (the previously cached values remain valid).
 *
 * On a communication error, retries up to 3 times with a 100 ms delay
 * between attempts to allow the sensor to recover.
 *
 * Context: Must be called with data->lock held.
 *
 * Return: 0 on success, negative errno on failure.
 */
static int dht11_do_read(struct dht11_data *data)
{
	u8 buf[DHT11_DATA_BYTES];
	int ret;
	int retries = 3;

	/* Rate limit: enforce minimum interval between readings */
	if (data->last_read &&
	    time_before(jiffies, data->last_read + DHT11_MIN_INTERVAL)) {
		dev_dbg(data->dev,
			"Reading too soon, returning cached values "
			"(temp=%d mC, hum=%d m%%)\n",
			data->temperature, data->humidity);
		return 0;  /* Return cached values */
	}

	while (retries-- > 0) {
		ret = dht11_read_raw_data(data, buf);
		if (ret) {
			dev_dbg(data->dev,
				"Communication error (attempt %d/3): %d\n",
				3 - retries, ret);
			msleep(100);
			continue;
		}

		ret = dht11_validate_checksum(buf);
		if (ret) {
			dev_warn(data->dev,
				 "Checksum mismatch (attempt %d/3): "
				 "[%02x %02x %02x %02x] checksum=%02x "
				 "expected=%02x\n",
				 3 - retries,
				 buf[0], buf[1], buf[2], buf[3], buf[4],
				 (buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF);
			msleep(100);
			continue;
		}

		/* Success — decode and cache */
		dht11_decode(data, buf);
		data->last_read = jiffies;

		dev_dbg(data->dev,
			"Read OK: temp=%d mC, humidity=%d m%%RH "
			"[raw: %02x %02x %02x %02x %02x]\n",
			data->temperature, data->humidity,
			buf[0], buf[1], buf[2], buf[3], buf[4]);

		return 0;
	}

	dev_err(data->dev,
		"Failed to read sensor after 3 attempts (last error: %d)\n",
		ret);
	return ret;
}

/* ------------------------------------------------------------------ */
/*  IIO subsystem integration                                          */
/* ------------------------------------------------------------------ */

/**
 * IIO channel definitions for the DHT11 sensor.
 *
 * Two channels are exposed:
 *   - IIO_TEMP channel: in_temp_raw (temperature in millidegrees C)
 *   - IIO_HUMIDITYRELATIVE channel: in_humidityrelative_raw (millipercent RH)
 *
 * Both channels only support IIO_CHAN_INFO_RAW (direct reading).
 * The scale factor is 1/1000 (raw is in millidegrees / millipercent),
 * applied via IIO_CHAN_INFO_SCALE so userspace can compute:
 *   processed_value = raw * scale
 */
static const struct iio_chan_spec dht11_channels[] = {
	{
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
				      BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_HUMIDITYRELATIVE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
				      BIT(IIO_CHAN_INFO_SCALE),
	},
};

/**
 * dht11_read_raw() - IIO read callback for the DHT11 sensor.
 * @indio_dev: IIO device structure.
 * @chan:       IIO channel being read.
 * @val:       Output value (integer part).
 * @val2:      Output value (fractional part, used for scale).
 * @mask:      Information mask (RAW or SCALE).
 *
 * For IIO_CHAN_INFO_RAW: triggers a sensor read (if the minimum interval
 * has elapsed) and returns the cached value in millidegrees C or
 * millipercent RH.
 *
 * For IIO_CHAN_INFO_SCALE: returns 0.001 (IIO_VAL_INT_PLUS_MICRO),
 * so that userspace computes: value_in_C = raw * 0.001
 *
 * Return: IIO_VAL_INT for raw reads, IIO_VAL_INT_PLUS_MICRO for scale,
 *         or negative errno on error.
 */
static int dht11_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2, long mask)
{
	struct dht11_data *data = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&data->lock);
		ret = dht11_do_read(data);
		if (ret) {
			mutex_unlock(&data->lock);
			return ret;
		}

		switch (chan->type) {
		case IIO_TEMP:
			*val = data->temperature;
			break;
		case IIO_HUMIDITYRELATIVE:
			*val = data->humidity;
			break;
		default:
			mutex_unlock(&data->lock);
			return -EINVAL;
		}
		mutex_unlock(&data->lock);

		return IIO_VAL_INT;

	case IIO_CHAN_INFO_SCALE:
		/*
		 * Scale factor: 0.001
		 * Raw values are in millidegrees C / millipercent RH.
		 * Userspace: processed = raw * 0.001
		 */
		*val = 0;
		*val2 = 1000;  /* 0.001 = 0 + 1000/1000000 */
		return IIO_VAL_INT_PLUS_MICRO;

	default:
		return -EINVAL;
	}
}

static const struct iio_info dht11_iio_info = {
	.read_raw = dht11_read_raw,
};

/* ------------------------------------------------------------------ */
/*  Platform driver probe / remove                                     */
/* ------------------------------------------------------------------ */

/**
 * dht11_probe() - Platform device probe callback.
 * @pdev: Platform device being bound.
 *
 * Allocates IIO device, acquires the GPIO, initialises driver state,
 * and registers the IIO device. After successful probe, the sensor
 * will be accessible at /sys/bus/iio/devices/iioX/.
 *
 * Return: 0 on success, negative errno on failure.
 */
static int dht11_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev;
	struct dht11_data *data;

	dev_info(dev, "%s: Probe function called\n", DRIVER_NAME);

	/*
	 * Allocate IIO device with private data embedded.
	 * iio_priv() is used later to retrieve the dht11_data pointer.
	 */
	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
	if (!indio_dev) {
		dev_err(dev, "%s: Failed to allocate IIO device\n",
			DRIVER_NAME);
		return -ENOMEM;
	}

	data = iio_priv(indio_dev);
	data->dev = dev;
	mutex_init(&data->lock);
	data->last_read = 0;
	data->temperature = 0;
	data->humidity = 0;

	/* Acquire GPIO from device tree — initially configured as input */
	data->gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(data->gpio)) {
		dev_err(dev, "%s: Failed to get GPIO from Device Tree: %ld\n",
			DRIVER_NAME, PTR_ERR(data->gpio));
		return PTR_ERR(data->gpio);
	}

	/* Configure the IIO device */
	indio_dev->name = DRIVER_NAME;
	indio_dev->info = &dht11_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = dht11_channels;
	indio_dev->num_channels = ARRAY_SIZE(dht11_channels);

	platform_set_drvdata(pdev, indio_dev);

	/* Register with the IIO subsystem */
	if (devm_iio_device_register(dev, indio_dev)) {
		dev_err(dev, "%s: Failed to register IIO device\n",
			DRIVER_NAME);
		return -ENODEV;
	}

	dev_info(dev, "%s: IIO device registered successfully "
		 "(channels: temp, humidity)\n", DRIVER_NAME);

	return 0;
}

/**
 * dht11_remove() - Platform device remove callback.
 * @pdev: Platform device being unbound.
 *
 * Since we use devm_* for all resource allocation, cleanup is
 * automatic. This function exists only for logging.
 *
 * Return: 0 always.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void dht11_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s: Driver removed\n", DRIVER_NAME);
}
#else
static int dht11_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s: Driver removed\n", DRIVER_NAME);
	return 0;
}
#endif

/* ------------------------------------------------------------------ */
/*  Device Tree matching                                               */
/* ------------------------------------------------------------------ */

static const struct of_device_id dht11_of_match[] = {
	{ .compatible = "industrial,dht11-iio", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dht11_of_match);

static struct platform_driver dht11_iio_driver = {
	.probe  = dht11_probe,
	.remove = dht11_remove,
	.driver = {
		.name           = DRIVER_NAME,
		.of_match_table = dht11_of_match,
	},
};

module_platform_driver(dht11_iio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imel23");
MODULE_DESCRIPTION("Industrial DHT11 IIO Driver with 1-wire protocol and checksum validation");
MODULE_VERSION("1.0");
