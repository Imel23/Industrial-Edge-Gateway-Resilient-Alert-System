export default function SensorChart({ title, data, color = 'brand' }) {
  const max = Math.max(...data)
  const min = Math.min(...data)
  const range = max - min || 1

  const colorMap = {
    brand: { bar: 'bg-brand-500', glow: 'shadow-brand-500/30' },
    info: { bar: 'bg-info-500', glow: 'shadow-info-500/30' },
    warning: { bar: 'bg-warning-500', glow: 'shadow-warning-500/30' },
  }

  const { bar, glow } = colorMap[color] || colorMap.brand

  return (
    <div className="glass rounded-2xl p-6 animate-fade-in">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-sm font-semibold text-slate-200">{title}</h3>
        <div className="flex items-center gap-2 text-xs text-slate-500">
          <span>Last 12 readings</span>
        </div>
      </div>

      {/* Simple bar chart */}
      <div className="flex items-end gap-1.5 h-32">
        {data.map((value, i) => {
          const height = ((value - min) / range) * 100
          return (
            <div
              key={i}
              className="flex-1 flex flex-col items-center gap-1 group"
            >
              <span className="text-[10px] text-slate-500 opacity-0 group-hover:opacity-100 transition-opacity">
                {value}
              </span>
              <div
                className={`w-full rounded-t-md ${bar} shadow-sm ${glow} transition-all duration-500 hover:opacity-80`}
                style={{
                  height: `${Math.max(height, 8)}%`,
                  animationDelay: `${i * 50}ms`,
                }}
              />
            </div>
          )
        })}
      </div>

      {/* X-axis labels */}
      <div className="flex justify-between mt-2 text-[10px] text-slate-600">
        <span>12 ago</span>
        <span>now</span>
      </div>
    </div>
  )
}
