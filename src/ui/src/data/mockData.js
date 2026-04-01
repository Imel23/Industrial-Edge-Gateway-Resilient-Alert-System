// ── Mock Data for Industrial Edge Gateway Dashboard ──────
// This file will be replaced with real API calls in a later sprint.

export const sensorReadings = {
  temperature: {
    value: 24.7,
    unit: '°C',
    min: 18.2,
    max: 31.5,
    status: 'normal',
    lastUpdate: '2026-04-01T19:15:00Z',
    history: [22.1, 22.5, 23.0, 23.8, 24.2, 24.7, 24.5, 24.3, 24.7, 25.0, 24.8, 24.7],
  },
  humidity: {
    value: 58.3,
    unit: '%',
    min: 35.0,
    max: 72.8,
    status: 'normal',
    lastUpdate: '2026-04-01T19:15:00Z',
    history: [55.0, 56.2, 57.1, 57.8, 58.0, 58.3, 58.1, 57.9, 58.3, 59.0, 58.5, 58.3],
  },
  sensorHealth: 'online',
  readingInterval: 2000, // ms
}

export const ledState = {
  isOn: true,
  mode: 'blink', // 'solid' | 'blink' | 'pulse'
  frequency: 5, // Hz (1-20)
  color: 'green',
  gpio: 'GPIO1_28',
  driverLoaded: true,
}

export const systemServices = [
  {
    id: 'daemon',
    name: 'System Daemon',
    description: 'Multithreaded C++ orchestration daemon',
    status: 'running',
    uptime: '2d 14h 32m',
    pid: 1247,
    cpu: 2.3,
    memory: 12.8,
  },
  {
    id: 'mqtt',
    name: 'MQTT Broker',
    description: 'Mosquitto MQTT message broker',
    status: 'running',
    uptime: '2d 14h 30m',
    pid: 1102,
    cpu: 0.8,
    memory: 8.4,
  },
  {
    id: 'api',
    name: 'API Server',
    description: 'FastAPI REST endpoint server',
    status: 'running',
    uptime: '1d 7h 15m',
    pid: 2341,
    cpu: 1.5,
    memory: 24.1,
  },
  {
    id: 'sensor-driver',
    name: 'DHT11 Driver',
    description: 'IIO subsystem kernel module',
    status: 'running',
    uptime: '2d 14h 32m',
    pid: null,
    cpu: 0.1,
    memory: 0.3,
  },
  {
    id: 'led-driver',
    name: 'LED Driver',
    description: 'GPIO LED kernel module with timer',
    status: 'running',
    uptime: '2d 14h 32m',
    pid: null,
    cpu: 0.0,
    memory: 0.1,
  },
  {
    id: 'button-driver',
    name: 'Button Driver',
    description: 'IRQ-based GPIO button kernel module',
    status: 'stopped',
    uptime: '—',
    pid: null,
    cpu: 0.0,
    memory: 0.0,
  },
]

export const systemMetrics = {
  cpu: {
    usage: 12.4,
    cores: 1,
    model: 'AM335x Cortex-A8',
    frequency: '1 GHz',
  },
  memory: {
    used: 187,
    total: 512,
    unit: 'MB',
  },
  disk: {
    used: 1.2,
    total: 4.0,
    unit: 'GB',
  },
  network: {
    ip: '192.168.1.42',
    interface: 'eth0',
    status: 'connected',
  },
  uptime: '2 days, 14 hours, 32 minutes',
  hostname: 'bbb-gateway',
  kernel: '6.6.32-yocto-standard',
}
