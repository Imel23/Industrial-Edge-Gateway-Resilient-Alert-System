import { BrowserRouter, Routes, Route } from 'react-router-dom'
import Layout from './components/layout/Layout'
import SensorDataPage from './pages/SensorDataPage'
import LedControlPage from './pages/LedControlPage'
import SystemStatusPage from './pages/SystemStatusPage'

export default function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route element={<Layout />}>
          <Route path="/" element={<SensorDataPage />} />
          <Route path="/led-control" element={<LedControlPage />} />
          <Route path="/system-status" element={<SystemStatusPage />} />
        </Route>
      </Routes>
    </BrowserRouter>
  )
}
