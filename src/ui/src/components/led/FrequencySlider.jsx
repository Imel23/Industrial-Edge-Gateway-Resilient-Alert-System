import { useState } from 'react'
import { Gauge } from 'lucide-react'

export default function FrequencySlider({ initialFrequency = 5, min = 1, max = 20, onChange }) {
  const [frequency, setFrequency] = useState(initialFrequency)

  const handleChange = (e) => {
    const val = Number(e.target.value)
    setFrequency(val)
    onChange?.(val)
  }

  const percentage = ((frequency - min) / (max - min)) * 100

  return (
    <div className="glass rounded-2xl p-6 animate-fade-in" style={{ animationDelay: '100ms' }}>
      <div className="flex items-center justify-between mb-6">
        <div className="flex items-center gap-3">
          <div className="p-2 rounded-xl bg-info-500/10 border border-info-500/20">
            <Gauge className="w-5 h-5 text-info-400" />
          </div>
          <h3 className="text-sm font-semibold text-slate-200">Blink Frequency</h3>
        </div>
        <span className="text-2xl font-bold text-white">{frequency}<span className="text-sm text-slate-400 ml-1">Hz</span></span>
      </div>

      {/* Slider */}
      <div className="space-y-3">
        <div className="relative">
          <input
            type="range"
            min={min}
            max={max}
            value={frequency}
            onChange={handleChange}
            className="w-full h-2 rounded-full appearance-none cursor-pointer bg-surface-700
              [&::-webkit-slider-thumb]:appearance-none
              [&::-webkit-slider-thumb]:w-5
              [&::-webkit-slider-thumb]:h-5
              [&::-webkit-slider-thumb]:rounded-full
              [&::-webkit-slider-thumb]:bg-white
              [&::-webkit-slider-thumb]:shadow-lg
              [&::-webkit-slider-thumb]:shadow-brand-500/30
              [&::-webkit-slider-thumb]:cursor-pointer
              [&::-webkit-slider-thumb]:transition-transform
              [&::-webkit-slider-thumb]:duration-200
              [&::-webkit-slider-thumb]:hover:scale-125
            "
            style={{
              background: `linear-gradient(to right, var(--color-brand-500) 0%, var(--color-brand-500) ${percentage}%, var(--color-surface-700) ${percentage}%, var(--color-surface-700) 100%)`,
            }}
          />
        </div>
        <div className="flex justify-between text-xs text-slate-500">
          <span>{min} Hz</span>
          <span>{max} Hz</span>
        </div>
      </div>

      {/* Preview */}
      <div className="mt-6 flex items-center gap-3">
        <span className="text-xs text-slate-400">Preview:</span>
        <div className="flex gap-1">
          {[...Array(5)].map((_, i) => (
            <div
              key={i}
              className="w-2 h-2 rounded-full bg-brand-500 animate-blink-led"
              style={{ animationDuration: `${1 / frequency}s`, animationDelay: `${i * 0.1}s` }}
            />
          ))}
        </div>
      </div>
    </div>
  )
}
