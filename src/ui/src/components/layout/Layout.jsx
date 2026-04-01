import { useState } from 'react'
import { Outlet, useLocation } from 'react-router-dom'
import Sidebar from './Sidebar'
import Header from './Header'

const pageTitles = {
  '/': 'Sensor Data',
  '/led-control': 'LED Control',
  '/system-status': 'System Status',
}

export default function Layout() {
  const [sidebarOpen, setSidebarOpen] = useState(true)
  const location = useLocation()
  const title = pageTitles[location.pathname] || 'Dashboard'

  return (
    <div className="min-h-screen bg-surface-950">
      <Sidebar
        isOpen={sidebarOpen}
        onToggle={() => setSidebarOpen(!sidebarOpen)}
      />

      {/* Main content area — shifts based on sidebar state */}
      <div
        className={`
          transition-all duration-300 ease-in-out min-h-screen
          ${sidebarOpen ? 'lg:ml-64' : 'lg:ml-20'}
        `}
      >
        <Header
          title={title}
          onMenuToggle={() => setSidebarOpen(!sidebarOpen)}
        />

        <main className="p-4 sm:p-6 lg:p-8">
          <Outlet />
        </main>
      </div>
    </div>
  )
}
