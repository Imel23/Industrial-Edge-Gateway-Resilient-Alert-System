import { useState } from 'react'
import { Zap, Settings, Cpu } from 'lucide-react'
import LedToggle from '../components/led/LedToggle'
import FrequencySlider from '../components/led/FrequencySlider'
import { ledState } from '../data/mockData'

const modes = [
  { id: 'solid', label: 'Solid', description: 'Continuous light' },
  { id: 'blink', label: 'Blink', description: 'On/off at set frequency' },
  { id: 'pulse', label: 'Pulse', description: 'Smooth fade in/out' },
]

export default function LedControlPage() {
  const [currentMode, setCurrentMode] = useState(ledState.mode)
  const [frequency, setFrequency] = useState(ledState.frequency)
  const [isOn, setIsOn] = useState(ledState.isOn)

  return (
    <div className="space-y-6">
      {/* Top row: Toggle + Frequency */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <LedToggle isOn={isOn} onToggle={setIsOn} />
        <FrequencySlider
          initialFrequency={frequency}
          onChange={setFrequency}
        />
      </div>

      {/* Mode Selector */}
      <div className="glass rounded-2xl p-6 animate-fade-in" style={{ animationDelay: '150ms' }}>
        <div className="flex items-center gap-3 mb-6">
          <div className="p-2 rounded-xl bg-warning-500/10 border border-warning-500/20">
            <Settings className="w-5 h-5 text-warning-400" />
          </div>
          <h3 className="text-sm font-semibold text-slate-200">LED Mode</h3>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-3 gap-3">
          {modes.map((mode) => (
            <button
              key={mode.id}
              onClick={() => setCurrentMode(mode.id)}
              className={`
                p-4 rounded-xl border transition-all duration-300 text-left cursor-pointer
                ${currentMode === mode.id
                  ? 'bg-brand-500/10 border-brand-500/30 shadow-lg shadow-brand-500/5'
                  : 'bg-surface-900/50 border-white/5 hover:border-white/10 hover:bg-surface-800/50'
                }
              `}
            >
              <p className={`text-sm font-semibold mb-1 ${
                currentMode === mode.id ? 'text-brand-400' : 'text-slate-300'
              }`}>
                {mode.label}
              </p>
              <p className="text-[11px] text-slate-500">{mode.description}</p>

              {/* Active indicator */}
              {currentMode === mode.id && (
                <div className="mt-3 flex items-center gap-1.5">
                  <div className="w-1.5 h-1.5 rounded-full bg-brand-500 animate-pulse" />
                  <span className="text-[10px] text-brand-400">Active</span>
                </div>
              )}
            </button>
          ))}
        </div>
      </div>

      {/* LED State Summary */}
      <div className="glass rounded-2xl p-6 animate-fade-in" style={{ animationDelay: '250ms' }}>
        <div className="flex items-center gap-3 mb-6">
          <div className="p-2 rounded-xl bg-brand-500/10 border border-brand-500/20">
            <Cpu className="w-5 h-5 text-brand-400" />
          </div>
          <h3 className="text-sm font-semibold text-slate-200">Current Configuration</h3>
        </div>

        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          {[
            { label: 'State', value: isOn ? 'ON' : 'OFF', color: isOn ? 'text-brand-400' : 'text-slate-400' },
            { label: 'Mode', value: currentMode.charAt(0).toUpperCase() + currentMode.slice(1), color: 'text-slate-200' },
            { label: 'Frequency', value: `${frequency} Hz`, color: 'text-info-400' },
            { label: 'GPIO', value: ledState.gpio, color: 'text-slate-200' },
          ].map((item, i) => (
            <div key={i} className="p-3 rounded-xl bg-surface-900/50">
              <p className="text-[11px] text-slate-500 mb-1">{item.label}</p>
              <p className={`text-sm font-bold ${item.color}`}>{item.value}</p>
            </div>
          ))}
        </div>

        {/* Driver info */}
        <div className="mt-4 pt-4 border-t border-white/5 flex items-center gap-2">
          <div className={`w-2 h-2 rounded-full ${ledState.driverLoaded ? 'bg-brand-500' : 'bg-danger-500'}`} />
          <span className="text-xs text-slate-400">
            LED Driver: {ledState.driverLoaded ? 'Loaded' : 'Not Loaded'} — industrial_led.ko
          </span>
        </div>
      </div>
    </div>
  )
}
