import { useState } from 'react'
import { NavLink, useLocation } from 'react-router-dom'
import {
  Thermometer,
  Lightbulb,
  Activity,
  ChevronLeft,
  ChevronRight,
  Radio,
} from 'lucide-react'

const navItems = [
  { path: '/', label: 'Sensor Data', icon: Thermometer },
  { path: '/led-control', label: 'LED Control', icon: Lightbulb },
  { path: '/system-status', label: 'System Status', icon: Activity },
]

export default function Sidebar({ isOpen, onToggle }) {
  const location = useLocation()

  return (
    <>
      {/* Mobile overlay */}
      {isOpen && (
        <div
          className="fixed inset-0 bg-black/50 backdrop-blur-sm z-40 lg:hidden"
          onClick={onToggle}
        />
      )}

      <aside
        className={`
          fixed top-0 left-0 h-full z-50
          transition-all duration-300 ease-in-out
          glass-strong flex flex-col
          ${isOpen ? 'w-64' : 'w-0 lg:w-20'}
          ${isOpen ? 'translate-x-0' : '-translate-x-full lg:translate-x-0'}
        `}
      >
        {/* Logo Section */}
        <div className="flex items-center gap-3 px-5 py-6 border-b border-white/5">
          <div className="flex-shrink-0 w-10 h-10 rounded-xl bg-gradient-to-br from-brand-500 to-brand-600 flex items-center justify-center shadow-lg shadow-brand-500/20">
            <Radio className="w-5 h-5 text-white" />
          </div>
          {isOpen && (
            <div className="animate-fade-in overflow-hidden">
              <h1 className="text-sm font-bold text-white whitespace-nowrap">Edge Gateway</h1>
              <p className="text-[10px] text-slate-400 whitespace-nowrap">Industrial IoT</p>
            </div>
          )}
        </div>

        {/* Navigation */}
        <nav className="flex-1 py-4 px-3 space-y-1 overflow-y-auto">
          {navItems.map((item) => {
            const Icon = item.icon
            const isActive = location.pathname === item.path

            return (
              <NavLink
                key={item.path}
                to={item.path}
                onClick={() => {
                  if (window.innerWidth < 1024) onToggle()
                }}
                className={`
                  group flex items-center gap-3 px-3 py-3 rounded-xl
                  transition-all duration-200 relative
                  ${isActive
                    ? 'bg-brand-500/15 text-brand-400'
                    : 'text-slate-400 hover:text-slate-200 hover:bg-white/5'
                  }
                `}
              >
                {/* Active indicator bar */}
                {isActive && (
                  <div className="absolute left-0 top-1/2 -translate-y-1/2 w-1 h-6 bg-brand-500 rounded-r-full animate-slide-in-left" />
                )}

                <Icon
                  className={`w-5 h-5 flex-shrink-0 transition-transform duration-200 group-hover:scale-110 ${
                    isActive ? 'text-brand-400' : ''
                  }`}
                />

                {isOpen && (
                  <span className="text-sm font-medium whitespace-nowrap animate-fade-in">
                    {item.label}
                  </span>
                )}
              </NavLink>
            )
          })}
        </nav>

        {/* Collapse toggle — desktop only */}
        <button
          onClick={onToggle}
          className="hidden lg:flex items-center justify-center py-4 border-t border-white/5 text-slate-500 hover:text-slate-300 transition-colors cursor-pointer"
        >
          {isOpen ? <ChevronLeft className="w-5 h-5" /> : <ChevronRight className="w-5 h-5" />}
        </button>
      </aside>
    </>
  )
}
