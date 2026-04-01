import { Thermometer, Droplets, Wifi, WifiOff, Clock } from 'lucide-react'
import SensorCard from '../components/sensor/SensorCard'
import SensorChart from '../components/sensor/SensorChart'
import { sensorReadings } from '../data/mockData'

export default function SensorDataPage() {
  const { temperature, humidity, sensorHealth, readingInterval } = sensorReadings

  const lastUpdate = new Date(temperature.lastUpdate).toLocaleTimeString('en-US', {
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
  })

  return (
    <div className="space-y-6">
      {/* Status bar */}
      <div className="flex flex-wrap items-center gap-4">
        <div className={`flex items-center gap-2 px-4 py-2 rounded-xl glass text-sm ${
          sensorHealth === 'online' ? 'text-brand-400' : 'text-danger-400'
        }`}>
          {sensorHealth === 'online' ? (
            <Wifi className="w-4 h-4" />
          ) : (
            <WifiOff className="w-4 h-4" />
          )}
          <span className="font-medium capitalize">Sensor {sensorHealth}</span>
        </div>

        <div className="flex items-center gap-2 px-4 py-2 rounded-xl glass text-sm text-slate-400">
          <Clock className="w-4 h-4" />
          <span>Last update: {lastUpdate}</span>
        </div>

        <div className="flex items-center gap-2 px-4 py-2 rounded-xl glass text-sm text-slate-400">
          <span>Interval: {readingInterval / 1000}s</span>
        </div>
      </div>

      {/* Sensor cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <SensorCard
          label="Temperature"
          value={temperature.value}
          unit={temperature.unit}
          min={temperature.min}
          max={temperature.max}
          status={temperature.status}
          icon={Thermometer}
          delay={0}
        />
        <SensorCard
          label="Humidity"
          value={humidity.value}
          unit={humidity.unit}
          min={humidity.min}
          max={humidity.max}
          status={humidity.status}
          icon={Droplets}
          delay={100}
        />
      </div>

      {/* Charts */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <SensorChart
          title="Temperature History"
          data={temperature.history}
          color="brand"
        />
        <SensorChart
          title="Humidity History"
          data={humidity.history}
          color="info"
        />
      </div>

      {/* DHT11 info card */}
      <div className="glass rounded-2xl p-6 animate-fade-in" style={{ animationDelay: '200ms' }}>
        <h3 className="text-sm font-semibold text-slate-200 mb-4">Sensor Information</h3>
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          {[
            { label: 'Sensor Model', value: 'DHT11' },
            { label: 'Interface', value: 'IIO Subsystem' },
            { label: 'GPIO Pin', value: 'GPIO1_13' },
            { label: 'Driver', value: 'dht11.ko' },
          ].map((item, i) => (
            <div key={i} className="p-3 rounded-xl bg-surface-900/50">
              <p className="text-[11px] text-slate-500 mb-1">{item.label}</p>
              <p className="text-sm font-medium text-slate-200">{item.value}</p>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}
