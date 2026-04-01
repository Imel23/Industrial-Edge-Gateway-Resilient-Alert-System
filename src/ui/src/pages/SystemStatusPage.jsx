import { Server, Cpu, HardDrive, MemoryStick, Globe, Clock } from 'lucide-react'
import ServiceCard from '../components/status/ServiceCard'
import StatusIndicator from '../components/status/StatusIndicator'
import { systemServices, systemMetrics } from '../data/mockData'

function MetricBar({ label, used, total, unit, color = 'brand' }) {
  const percentage = (used / total) * 100
  const colorMap = {
    brand: 'from-brand-500 to-brand-400',
    info: 'from-info-500 to-info-400',
    warning: 'from-warning-500 to-warning-400',
  }

  return (
    <div className="space-y-2">
      <div className="flex justify-between items-baseline">
        <span className="text-sm text-slate-300">{label}</span>
        <span className="text-sm font-semibold text-white">
          {used} <span className="text-slate-500 font-normal">/ {total} {unit}</span>
        </span>
      </div>
      <div className="h-2 bg-surface-700 rounded-full overflow-hidden">
        <div
          className={`h-full rounded-full bg-gradient-to-r ${colorMap[color]} transition-all duration-700`}
          style={{ width: `${percentage}%` }}
        />
      </div>
      <p className="text-[11px] text-slate-500 text-right">{percentage.toFixed(1)}%</p>
    </div>
  )
}

export default function SystemStatusPage() {
  const runningCount = systemServices.filter(s => s.status === 'running').length
  const totalCount = systemServices.length

  return (
    <div className="space-y-6">
      {/* Overview bar */}
      <div className="flex flex-wrap gap-4">
        <div className="flex items-center gap-3 px-4 py-2.5 rounded-xl glass">
          <Server className="w-4 h-4 text-brand-400" />
          <span className="text-sm text-slate-300">
            <span className="font-semibold text-brand-400">{runningCount}</span>
            <span className="text-slate-500">/{totalCount}</span> services running
          </span>
        </div>
        <div className="flex items-center gap-3 px-4 py-2.5 rounded-xl glass">
          <Clock className="w-4 h-4 text-slate-400" />
          <span className="text-sm text-slate-300">{systemMetrics.uptime}</span>
        </div>
        <div className="flex items-center gap-3 px-4 py-2.5 rounded-xl glass">
          <Globe className="w-4 h-4 text-info-400" />
          <span className="text-sm text-slate-300">{systemMetrics.network.ip}</span>
        </div>
      </div>

      {/* System Metrics */}
      <div className="glass rounded-2xl p-6 animate-fade-in">
        <div className="flex items-center gap-3 mb-6">
          <div className="p-2 rounded-xl bg-info-500/10 border border-info-500/20">
            <Cpu className="w-5 h-5 text-info-400" />
          </div>
          <div>
            <h3 className="text-sm font-semibold text-slate-200">System Resources</h3>
            <p className="text-[11px] text-slate-500">
              {systemMetrics.cpu.model} • {systemMetrics.hostname}
            </p>
          </div>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-3 gap-6">
          <div>
            <div className="flex items-center gap-2 mb-3">
              <Cpu className="w-4 h-4 text-brand-400" />
              <span className="text-xs font-medium text-slate-400">CPU</span>
            </div>
            <div className="text-center p-4 rounded-xl bg-surface-900/50">
              <p className="text-3xl font-bold text-white mb-1">{systemMetrics.cpu.usage}%</p>
              <p className="text-[11px] text-slate-500">{systemMetrics.cpu.cores} core @ {systemMetrics.cpu.frequency}</p>
            </div>
          </div>

          <MetricBar
            label="Memory"
            used={systemMetrics.memory.used}
            total={systemMetrics.memory.total}
            unit={systemMetrics.memory.unit}
            color="info"
          />
          <MetricBar
            label="Disk"
            used={systemMetrics.disk.used}
            total={systemMetrics.disk.total}
            unit={systemMetrics.disk.unit}
            color="warning"
          />
        </div>
      </div>

      {/* Service Cards */}
      <div>
        <h3 className="text-sm font-semibold text-slate-300 mb-4">Services & Drivers</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-4">
          {systemServices.map((service, i) => (
            <ServiceCard key={service.id} service={service} delay={i * 80} />
          ))}
        </div>
      </div>

      {/* System Info */}
      <div className="glass rounded-2xl p-6 animate-fade-in" style={{ animationDelay: '300ms' }}>
        <h3 className="text-sm font-semibold text-slate-200 mb-4">System Information</h3>
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          {[
            { label: 'Hostname', value: systemMetrics.hostname },
            { label: 'Kernel', value: systemMetrics.kernel },
            { label: 'Network', value: `${systemMetrics.network.interface} (${systemMetrics.network.status})` },
            { label: 'IP Address', value: systemMetrics.network.ip },
          ].map((item, i) => (
            <div key={i} className="p-3 rounded-xl bg-surface-900/50">
              <p className="text-[11px] text-slate-500 mb-1">{item.label}</p>
              <p className="text-xs font-medium text-slate-200 break-all">{item.value}</p>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}
