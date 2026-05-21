"use client";

import { useState } from "react";
import { Wrench, Zap, CheckCircle2, RefreshCw, AlertCircle, ShoppingCart } from "lucide-react";
import { generateAffiliateLink } from "@/lib/affiliate";

type HardwareItem = {
  name: string;
  link: string;
  mpn: string;
  status: "ok" | "checking" | "error";
  warning?: string;
};

const TRANSMITTER_HW: HardwareItem[] = [
  { name: "DIYmall LoRa32u4 RA-02 (433MHz)", link: "https://es.aliexpress.com/item/1005010740065797.html", mpn: "LORA32U4-SX1278-433", status: "ok" },
  { name: "LiPo Battery 3.7V 1500mAh 103048", link: "https://es.aliexpress.com/item/1005011524169071.html", mpn: "LIPO-103048-1500", status: "ok" },
  { name: "VL53L4CD ToF Distance Sensor", link: "https://es.aliexpress.com/item/1005011974180379.html", mpn: "VL53L4CD-TOF", status: "ok" },
  { name: "KW12 Limit Switch (18mm lever)", link: "https://es.aliexpress.com/item/32812520453.html", mpn: "LIMIT-SWITCH-KW12", status: "ok" },
  { name: "304 Stainless V-Type Torsion Springs", link: "https://es.aliexpress.com/w/wholesale-torsion-spring-1.0mm-stainless-steel.html", mpn: "SPRING-V-INOX-304", status: "ok" },
  { name: "GX12 5-Pin Male Connector", link: "https://es.aliexpress.com/item/1005006162408269.html", mpn: "GX12-5PIN-M", status: "ok" },
  { name: "22AWG Silicone Wire Kit (5 Colors)", link: "https://es.aliexpress.com/item/1005008256201450.html", mpn: "22AWG-SIL-KIT-5", status: "ok" },
  { name: "M16 Blue Aluminum LED Button", link: "https://es.aliexpress.com/item/1005007191696739.html", mpn: "M16-LED-PUSH-BUTTON", status: "ok" }
];

const RECEIVER_HW: HardwareItem[] = [
  { name: "DIYmall LoRa32u4 RA-02 (433MHz)", link: "https://es.aliexpress.com/item/1005010740065797.html", mpn: "LORA32U4-SX1278-433", status: "ok" },
  { name: "LiPo Battery 3.7V 1500mAh 103048", link: "https://es.aliexpress.com/item/1005011524169071.html", mpn: "LIPO-103048-1500", status: "ok" },
  { 
    name: "GX12 5-Pin Male Connector", 
    link: "https://es.aliexpress.com/item/1005006162408269.html", 
    mpn: "GX12-5PIN-M", 
    status: "ok",
    warning: "Remember to choose the '5Pin Male' version (GX12-5) to ensure compatibility with standard welder ports."
  },
  { 
    name: "Photocoupler TLP222A (x2)", 
    link: "https://es.aliexpress.com/item/1005009404744214.html", 
    mpn: "TLP222A-PHOTO", 
    status: "ok",
    warning: "The TLP222A is a 4-pin single-channel photorelay. Remember to buy two units (one for torch trigger, one for DAC isolation)."
  },
  { name: "0.96 inch OLED 128x64 SSD1306", link: "https://es.aliexpress.com/item/1005006141235306.html", mpn: "SSD1306-OLED-0.96", status: "ok" },
  { 
    name: "PWM-to-DAC Module (0-10V)", 
    link: "https://es.aliexpress.com/item/1005006246628087.html", 
    mpn: "PWM-TO-DAC-0-10V", 
    status: "ok",
    warning: "Remember to choose the 'Analog Input Interface' or 'Analog Remote Control Port' (0-10V). *Please verify with your welder specifications."
  },
  { name: "22AWG Silicone Wire Kit (5 Colors)", link: "https://es.aliexpress.com/item/1005008256201450.html", mpn: "22AWG-SIL-KIT-5", status: "ok" },
  { name: "M16 Blue Aluminum LED Button", link: "https://es.aliexpress.com/item/1005007191696739.html", mpn: "M16-LED-PUSH-BUTTON", status: "ok" }
];

export default function HardwareSpecs() {
  const [transmitterHw, setTransmitterHw] = useState(TRANSMITTER_HW);
  const [receiverHw, setReceiverHw] = useState(RECEIVER_HW);
  const [isChecking, setIsChecking] = useState(false);

  const handlePurchaseClick = (e: React.MouseEvent, item: HardwareItem) => {
    if (item.warning) {
      e.preventDefault();
      alert(`⚠️ ATTENTION:\n\n${item.warning}`);
      window.open(generateAffiliateLink(item.link), '_blank');
    }
  };

  const checkHealth = async () => {
    setIsChecking(true);
    setTimeout(() => {
      setTransmitterHw(prev => prev.map(item => ({ ...item, status: "ok" })));
      setReceiverHw(prev => prev.map(item => ({ ...item, status: "ok" })));
      setIsChecking(false);
    }, 1500);
  };

  return (
    <section id="hardware" className="py-32 px-4 bg-gradient-to-r from-blue-600 to-cyan-500 relative overflow-hidden">
      {/* Subtle overlay to soften the gradient if needed */}
      <div className="absolute inset-0 bg-black/5 pointer-events-none" />
      
      <div className="max-w-6xl mx-auto relative z-10">
        <div className="flex flex-col md:flex-row justify-between items-end mb-20 gap-8">
          <div className="text-left">
            <h2 className="text-4xl md:text-5xl font-black mb-6 tracking-tight text-white outline-none">
              Hardware & Assembly <span className="text-white/70 font-light block md:inline text-3xl">(Interactive BOM)</span>
            </h2>
            <p className="text-xl text-white/90 max-w-2xl font-medium">
              Interactive lists synced with global stock. Click any component to buy with your tracking ID.
            </p>
          </div>
          <button 
            onClick={checkHealth}
            disabled={isChecking}
            className="flex items-center gap-2 px-8 py-4 bg-white text-blue-600 rounded-2xl hover:bg-blue-50 transition-all font-bold shadow-2xl disabled:opacity-50"
          >
            <RefreshCw className={`w-5 h-5 ${isChecking ? "animate-spin" : ""}`} />
            Check Link Health
          </button>
        </div>

        <div className="grid md:grid-cols-2 gap-12 lg:gap-16">
          {/* TRANSMITTER (PEDAL) */}
          <div className="relative group">
            <div className="bg-white p-10 md:p-12 rounded-[40px] text-neutral-900 shadow-2xl relative overflow-hidden group h-full flex flex-col transition-all hover:scale-[1.01] border border-neutral-100">
              <div className="absolute top-0 right-0 w-32 h-32 bg-blue-500/5 blur-3xl -mr-16 -mt-16 group-hover:bg-blue-500/10 transition-colors" />
              
              <div className="relative z-10 flex flex-col h-full">
                <div className="flex items-center gap-4 mb-10 text-blue-600">
                  <div className="p-3 bg-blue-500/10 rounded-2xl">
                    <Zap className="w-8 h-8" />
                  </div>
                  <h3 className="text-3xl font-black tracking-tighter text-neutral-900">Transmitter <span className="text-neutral-400 font-light">(Pedal)</span></h3>
                </div>
                
                <div className="mb-10 space-y-6">
                  <h4 className="text-xs font-bold uppercase tracking-widest text-neutral-400 border-b border-neutral-100 pb-2">Hardware List (BOM)</h4>
                  <ul className="space-y-4">
                    {transmitterHw.map((item, i) => (
                      <li key={i} className="flex items-start justify-between gap-3 group/item text-sm">
                        <div className="flex items-center gap-3">
                          <CheckCircle2 className={`w-5 h-5 shrink-0 ${item.status === 'ok' ? 'text-blue-500' : 'text-amber-500'}`} />
                          <a 
                            href={generateAffiliateLink(item.link)}
                            onClick={(e) => handlePurchaseClick(e, item)}
                            target="_blank"
                            rel="noopener noreferrer"
                            className="font-bold text-neutral-800 hover:text-blue-600 border-b border-transparent hover:border-blue-600 transition-all flex items-center gap-2"
                          >
                            {item.name}
                            <ShoppingCart className="w-3 h-3 opacity-0 group-hover/item:opacity-100 transition-opacity" />
                          </a>
                        </div>
                        <span className="text-[10px] font-mono text-neutral-400 uppercase font-medium">{item.mpn}</span>
                      </li>
                    ))}
                    <li className="flex items-center gap-3 italic text-neutral-400 text-xs">
                      <CheckCircle2 className="w-4 h-4 shrink-0 opacity-30" />
                      3D Printed Parts (Pedal)
                    </li>
                  </ul>
                </div>

                <div className="mt-auto space-y-6 bg-neutral-50 p-7 rounded-[30px] shadow-inner">
                  <h4 className="text-xs font-bold uppercase tracking-widest text-neutral-400">Assembly Steps</h4>
                  <div className="space-y-4 text-sm text-neutral-600 leading-relaxed">
                    <p><span className="font-bold text-neutral-900">1. Flashing:</span> Upload the `.ino` firmware to the LoRa32u4 module via USB using Arduino IDE.</p>
                    <p><span className="font-bold text-neutral-900">2. Schematics:</span> Connect the VL53L4CD sensor (I2C) and the Limit-Switch KW12 (GND/Digital pin).</p>
                    <p><span className="font-bold text-neutral-900">3. Assembly:</span> Install the Inox 304 torsion springs for pedal reset and mount the GX12 connector for external output.</p>
                  </div>
                </div>
              </div>
            </div>
          </div>

          {/* RECEIVER (WELDER SIDE) */}
          <div className="relative group">
            <div className="bg-white p-10 md:p-12 rounded-[40px] text-neutral-900 shadow-2xl relative overflow-hidden group h-full flex flex-col transition-all hover:scale-[1.01] border border-neutral-100">
              <div className="absolute top-0 right-0 w-32 h-32 bg-cyan-500/5 blur-3xl -mr-16 -mt-16 group-hover:bg-cyan-500/10 transition-colors" />
              
              <div className="relative z-10 flex flex-col h-full">
                <div className="flex items-center gap-4 mb-10 text-cyan-600">
                  <div className="p-3 bg-cyan-500/10 rounded-2xl">
                    <Wrench className="w-8 h-8" />
                  </div>
                  <h3 className="text-3xl font-black tracking-tighter text-neutral-900">Receiver <span className="text-neutral-400 font-light">(Welder)</span></h3>
                </div>
                
                <div className="mb-10 space-y-6">
                  <h4 className="text-xs font-bold uppercase tracking-widest text-neutral-400 border-b border-neutral-100 pb-2">Hardware List (BOM)</h4>
                  <ul className="space-y-4">
                    {receiverHw.map((item, i) => (
                      <li key={i} className="flex items-start justify-between gap-3 group/item text-sm">
                        <div className="flex items-center gap-3">
                          <CheckCircle2 className={`w-5 h-5 shrink-0 ${item.status === 'ok' ? 'text-cyan-500' : 'text-amber-500'}`} />
                          <a 
                            href={generateAffiliateLink(item.link)}
                            onClick={(e) => handlePurchaseClick(e, item)}
                            target="_blank"
                            rel="noopener noreferrer"
                            className="font-bold text-neutral-800 hover:text-cyan-600 border-b border-transparent hover:border-cyan-600 transition-all flex items-center gap-2"
                          >
                            {item.name}
                            <ShoppingCart className="w-3 h-3 opacity-0 group-hover/item:opacity-100 transition-opacity" />
                          </a>
                        </div>
                        <span className="text-[10px] font-mono text-neutral-400 uppercase font-medium">{item.mpn}</span>
                      </li>
                    ))}
                  </ul>
                </div>

                <div className="mt-auto space-y-6 bg-neutral-50 p-7 rounded-[30px] shadow-inner">
                  <h4 className="text-xs font-bold uppercase tracking-widest text-neutral-400">Assembly Steps</h4>
                  <div className="space-y-4 text-sm text-neutral-600 leading-relaxed">
                    <p><span className="font-bold text-neutral-900">1. Flashing:</span> Upload the receiver `.ino` firmware to the second LoRa32u4 module.</p>
                    <p><span className="font-bold text-neutral-900">2. Interface:</span> Connect the SSD1306 OLED display (I2C) to monitor telemetry and system status.</p>
                    <p><span className="font-bold text-neutral-900">3. Logic:</span> Integrate the TLP222A-2 photocoupler and PWM-to-DAC module with the GX12 aviation port.</p>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
