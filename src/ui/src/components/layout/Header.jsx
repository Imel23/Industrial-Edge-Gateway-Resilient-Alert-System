import { Menu, Bell, Wifi } from 'lucide-react'

export default function Header({ title, onMenuToggle }) {
  return (
    <header className="sticky top-0 z-30 glass border-b border-white/5">
      <div className="flex items-center justify-between px-4 sm:px-6 py-4">
        {/* Left: Menu + Title */}
        <div className="flex items-center gap-4">
          <button
            onClick={onMenuToggle}
            className="lg:hidden p-2 rounded-lg text-slate-400 hover:text-white hover:bg-white/5 transition-colors cursor-pointer"
            aria-label="Toggle menu"
          >
            <Menu className="w-5 h-5" />
          </button>

          <div>
            <h2 className="text-lg font-semibold text-white">{title}</h2>
            <p className="text-xs text-slate-500 hidden sm:block">
              BeagleBone Black • bbb-gateway
            </p>
          </div>
        </div>

        {/* Right: Status indicators */}
        <div className="flex items-center gap-3">
          {/* Connection status */}
          <div className="flex items-center gap-2 px-3 py-1.5 rounded-full glass text-xs">
            <Wifi className="w-3.5 h-3.5 text-brand-400" />
            <span className="text-slate-300 hidden sm:inline">Connected</span>
            <span className="w-2 h-2 rounded-full bg-brand-500 animate-pulse" />
          </div>

          {/* Notifications */}
          <button className="relative p-2 rounded-lg text-slate-400 hover:text-white hover:bg-white/5 transition-colors cursor-pointer">
            <Bell className="w-5 h-5" />
            <span className="absolute top-1 right-1 w-2 h-2 rounded-full bg-danger-500" />
          </button>
        </div>
      </div>
    </header>
  )
}
