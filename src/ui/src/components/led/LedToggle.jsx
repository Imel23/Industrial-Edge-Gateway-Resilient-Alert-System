import { useState } from 'react'
import { Power } from 'lucide-react'

export default function LedToggle({ isOn: initialIsOn, onToggle }) {
  const [isOn, setIsOn] = useState(initialIsOn)

  const handleToggle = () => {
    setIsOn(!isOn)
    onToggle?.(!isOn)
  }

  return (
    <div className="glass rounded-2xl p-6 animate-fade-in">
      <div className="flex items-center justify-between mb-6">
        <h3 className="text-sm font-semibold text-slate-200">Power Control</h3>
        <span className={`text-xs font-medium px-2.5 py-1 rounded-full ${
          isOn
            ? 'bg-brand-500/15 text-brand-400'
            : 'bg-slate-500/15 text-slate-400'
        }`}>
          {isOn ? 'Active' : 'Inactive'}
        </span>
      </div>

      {/* LED visual indicator */}
      <div className="flex flex-col items-center gap-6 py-4">
        <div className={`
          w-24 h-24 rounded-full flex items-center justify-center
          transition-all duration-500
          ${isOn
            ? 'bg-brand-500/20 shadow-[0_0_40px_theme(--color-brand-500/0.3)] animate-pulse-glow'
            : 'bg-surface-700/50'
          }
        `}>
          <div className={`
            w-16 h-16 rounded-full flex items-center justify-center
            transition-all duration-500
            ${isOn
              ? 'bg-gradient-to-br from-brand-400 to-brand-600 shadow-lg shadow-brand-500/40'
              : 'bg-surface-600'
            }
          `}>
            <Power className={`w-7 h-7 transition-colors duration-300 ${
              isOn ? 'text-white' : 'text-slate-500'
            }`} />
          </div>
        </div>

        {/* Toggle switch */}
        <button
          onClick={handleToggle}
          className={`
            relative w-16 h-8 rounded-full transition-all duration-300 cursor-pointer
            ${isOn
              ? 'bg-brand-500 shadow-lg shadow-brand-500/30'
              : 'bg-surface-600'
            }
          `}
        >
          <div className={`
            absolute top-1 w-6 h-6 rounded-full bg-white shadow-md
            transition-all duration-300
            ${isOn ? 'left-9' : 'left-1'}
          `} />
        </button>
      </div>
    </div>
  )
}
