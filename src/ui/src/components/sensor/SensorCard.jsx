import { TrendingUp, TrendingDown, Minus } from 'lucide-react'

export default function SensorCard({ label, value, unit, min, max, status, icon: Icon, delay = 0 }) {
  const trend = value > (min + max) / 2 ? 'up' : value < (min + max) / 2 ? 'down' : 'neutral'

  const statusColors = {
    normal: 'text-brand-400',
    warning: 'text-warning-400',
    critical: 'text-danger-400',
  }

  const statusBgColors = {
    normal: 'bg-brand-500/10 border-brand-500/20',
    warning: 'bg-warning-500/10 border-warning-500/20',
    critical: 'bg-danger-500/10 border-danger-500/20',
  }

  return (
    <div
      className="glass rounded-2xl p-6 hover:bg-surface-800/80 transition-all duration-300 hover:scale-[1.02] hover:shadow-lg hover:shadow-brand-500/5 animate-fade-in group"
      style={{ animationDelay: `${delay}ms` }}
    >
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-3">
          <div className={`p-2.5 rounded-xl ${statusBgColors[status]} border`}>
            <Icon className={`w-5 h-5 ${statusColors[status]}`} />
          </div>
          <div>
            <p className="text-sm font-medium text-slate-300">{label}</p>
            <p className="text-[11px] text-slate-500 capitalize">{status}</p>
          </div>
        </div>

        {/* Trend indicator */}
        <div className={`p-1.5 rounded-lg ${
          trend === 'up' ? 'bg-brand-500/10 text-brand-400' :
          trend === 'down' ? 'bg-info-500/10 text-info-400' :
          'bg-slate-500/10 text-slate-400'
        }`}>
          {trend === 'up' && <TrendingUp className="w-4 h-4" />}
          {trend === 'down' && <TrendingDown className="w-4 h-4" />}
          {trend === 'neutral' && <Minus className="w-4 h-4" />}
        </div>
      </div>

      {/* Value */}
      <div className="mb-4">
        <span className="text-4xl font-bold text-white tracking-tight group-hover:text-brand-400 transition-colors duration-300">
          {value}
        </span>
        <span className="text-lg text-slate-400 ml-1">{unit}</span>
      </div>

      {/* Min / Max bar */}
      <div className="space-y-2">
        <div className="flex justify-between text-xs text-slate-500">
          <span>Min: {min}{unit}</span>
          <span>Max: {max}{unit}</span>
        </div>
        <div className="h-1.5 bg-surface-700 rounded-full overflow-hidden">
          <div
            className="h-full rounded-full bg-gradient-to-r from-info-500 via-brand-500 to-warning-500 transition-all duration-700"
            style={{ width: `${((value - min) / (max - min)) * 100}%` }}
          />
        </div>
      </div>
    </div>
  )
}
