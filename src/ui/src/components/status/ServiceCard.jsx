import StatusIndicator from './StatusIndicator'
import { Clock, Cpu, HardDrive } from 'lucide-react'

export default function ServiceCard({ service, delay = 0 }) {
  const statusLabels = {
    running: 'Running',
    stopped: 'Stopped',
    error: 'Error',
  }

  const statusBadgeColors = {
    running: 'bg-brand-500/10 text-brand-400 border-brand-500/20',
    stopped: 'bg-slate-500/10 text-slate-400 border-slate-500/20',
    error: 'bg-danger-500/10 text-danger-400 border-danger-500/20',
  }

  return (
    <div
      className="glass rounded-2xl p-5 hover:bg-surface-800/80 transition-all duration-300 hover:scale-[1.02] hover:shadow-lg animate-fade-in group"
      style={{ animationDelay: `${delay}ms` }}
    >
      {/* Header */}
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center gap-3">
          <StatusIndicator status={service.status} size="md" />
          <div>
            <h4 className="text-sm font-semibold text-white group-hover:text-brand-400 transition-colors">
              {service.name}
            </h4>
            <p className="text-[11px] text-slate-500">{service.description}</p>
          </div>
        </div>

        <span className={`text-[11px] font-medium px-2.5 py-1 rounded-full border ${statusBadgeColors[service.status]}`}>
          {statusLabels[service.status]}
        </span>
      </div>

      {/* Metrics */}
      <div className="grid grid-cols-3 gap-3 mt-4 pt-4 border-t border-white/5">
        <div className="flex items-center gap-2">
          <Clock className="w-3.5 h-3.5 text-slate-500" />
          <div>
            <p className="text-[10px] text-slate-500">Uptime</p>
            <p className="text-xs font-medium text-slate-300">{service.uptime}</p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <Cpu className="w-3.5 h-3.5 text-slate-500" />
          <div>
            <p className="text-[10px] text-slate-500">CPU</p>
            <p className="text-xs font-medium text-slate-300">{service.cpu}%</p>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <HardDrive className="w-3.5 h-3.5 text-slate-500" />
          <div>
            <p className="text-[10px] text-slate-500">Memory</p>
            <p className="text-xs font-medium text-slate-300">{service.memory}%</p>
          </div>
        </div>
      </div>

      {/* PID */}
      {service.pid && (
        <div className="mt-3 text-[10px] text-slate-600">
          PID: {service.pid}
        </div>
      )}
    </div>
  )
}
