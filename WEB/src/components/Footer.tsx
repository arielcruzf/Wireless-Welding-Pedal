import { GitBranch, ShoppingCart, Box, HelpCircle, Heart, AlertTriangle } from "lucide-react";

export default function Footer() {
  return (
    <footer className="bg-neutral-950 text-neutral-400 py-20 px-4 border-t border-neutral-800">
      <div className="max-w-7xl mx-auto">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-12 mb-16">
          
          {/* Column 1: Project Info */}
          <div className="space-y-6">
            <h3 className="text-white text-xl font-bold tracking-tight">Wireless Welder Pedal</h3>
            <p className="text-sm leading-relaxed">
              An Open Source project dedicated to improving ergonomics and precision in TIG/MMA welding through DIY wireless control.
            </p>
            <div className="flex items-center gap-2 text-amber-500 bg-amber-500/10 p-3 rounded-xl border border-amber-500/20 text-xs">
              <AlertTriangle className="w-4 h-4 shrink-0" />
              <span>Safety focus: Proceed with caution when handling industrial voltages.</span>
            </div>
          </div>

          {/* Column 2: Hardware (AliExpress) */}
          <div className="space-y-6">
            <h4 className="text-white font-semibold text-sm uppercase tracking-widest">Hardware (AliExpress)</h4>
            <ul className="space-y-3 text-sm">
              <li>
                <a href="https://es.aliexpress.com/item/1005010740065797.html" target="_blank" className="hover:text-blue-400 transition-colors flex items-center gap-2">
                  <ShoppingCart className="w-4 h-4" /> LoRa32u4 RA-02 Module
                </a>
              </li>
              <li>
                <a href="https://es.aliexpress.com/item/1005011974180379.html" target="_blank" className="hover:text-blue-400 transition-colors flex items-center gap-2">
                  <ShoppingCart className="w-4 h-4" /> VL53L4CD Laser Sensor
                </a>
              </li>
              <li>
                <a href="https://es.aliexpress.com/item/1005008640108394.html" target="_blank" className="hover:text-blue-400 transition-colors flex items-center gap-2">
                  <ShoppingCart className="w-4 h-4" /> OLED Display SSD1306
                </a>
              </li>
              <li>
                <a href="https://es.aliexpress.com/item/1005010491208213.html" target="_blank" className="hover:text-blue-400 transition-colors flex items-center gap-2">
                  <ShoppingCart className="w-4 h-4" /> Isolated MOSFET Module
                </a>
              </li>
              <li>
                <a href="https://es.aliexpress.com/item/1005006246628087.html" target="_blank" className="hover:text-blue-400 transition-colors flex items-center gap-2">
                  <ShoppingCart className="w-4 h-4" /> PWM to DAC Converter
                </a>
              </li>
            </ul>
          </div>

          {/* Column 3: 3D Services */}
          <div className="space-y-6">
            <h4 className="text-white font-semibold text-sm uppercase tracking-widest">3D Printing Services</h4>
            <ul className="space-y-3 text-sm">
              <li>
                <a href="https://jlcpcb.com/3d-printing" target="_blank" className="hover:text-cyan-400 transition-colors flex items-center gap-2">
                  <Box className="w-4 h-4" /> JLCPCB (3D Printing)
                </a>
              </li>
              <li>
                <a href="https://www.shapeways.com/" target="_blank" className="hover:text-cyan-400 transition-colors flex items-center gap-2">
                  <Box className="w-4 h-4" /> Shapeways Industrial
                </a>
              </li>
              <li>
                <a href="https://www.pcbway.com/rapid-prototyping/3d-printing/" target="_blank" className="hover:text-cyan-400 transition-colors flex items-center gap-2">
                  <Box className="w-4 h-4" /> PCBWay Manufacturing
                </a>
              </li>
            </ul>
          </div>

          {/* Column 4: Links & Docs */}
          <div className="space-y-6">
            <h4 className="text-white font-semibold text-sm uppercase tracking-widest">Community</h4>
            <ul className="space-y-3 text-sm">
              <li>
                <a href="https://github.com" target="_blank" className="hover:text-white transition-colors flex items-center gap-2">
                  <GitBranch className="w-4 h-4" /> GitHub Repository
                </a>
              </li>
              <li>
                <a href="#support" className="hover:text-white transition-colors flex items-center gap-2">
                  <HelpCircle className="w-4 h-4" /> RAG Documentation
                </a>
              </li>
              <li className="pt-4">
                <div className="p-4 bg-neutral-900 rounded-2xl border border-neutral-800 flex items-center gap-3">
                  <div className="w-10 h-10 bg-blue-500/10 rounded-full flex items-center justify-center text-blue-500">
                    <Heart className="w-5 h-5" />
                  </div>
                  <div>
                    <div className="text-white text-xs font-bold">Support Us</div>
                    <div className="text-[10px]">Open Source Hardware</div>
                  </div>
                </div>
              </li>
            </ul>
          </div>

        </div>

        {/* Bottom Bar */}
        <div className="pt-8 border-t border-neutral-900 flex flex-col md:flex-row justify-between items-center gap-4 text-xs">
          <p>© 2026 Wireless Welder Pedal Project. Licensed under MIT.</p>
          <div className="flex gap-8">
            <a href="#" className="hover:text-white">Privacy Policy</a>
            <a href="#" className="hover:text-white">Terms of Use</a>
            <a href="#" className="hover:text-white">Cookies</a>
          </div>
        </div>
      </div>
    </footer>
  );
}
