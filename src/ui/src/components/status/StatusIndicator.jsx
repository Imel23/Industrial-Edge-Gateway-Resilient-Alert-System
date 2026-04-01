export default function StatusIndicator({ status, size = 'md' }) {
  const sizeMap = {
    sm: 'w-2 h-2',
    md: 'w-3 h-3',
    lg: 'w-4 h-4',
  }

  const colorMap = {
    running: 'bg-brand-500',
    stopped: 'bg-slate-500',
    error: 'bg-danger-500',
    warning: 'bg-warning-500',
  }

  const glowMap = {
    running: 'shadow-brand-500/50',
    stopped: '',
    error: 'shadow-danger-500/50',
    warning: 'shadow-warning-500/50',
  }

  return (
    <span className="relative flex items-center justify-center">
      {/* Pulse ring for active statuses */}
      {(status === 'running' || status === 'error') && (
        <span
          className={`absolute ${sizeMap[size]} rounded-full ${colorMap[status]} opacity-40 animate-ping`}
        />
      )}
      <span
        className={`relative ${sizeMap[size]} rounded-full ${colorMap[status]} shadow-sm ${glowMap[status]}`}
      />
    </span>
  )
}
