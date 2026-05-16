import { Download, FileCode, Printer, Map, Shield, HelpCircle, ArrowRight } from "lucide-react";

export default function PurchaseSection() {
  const features = [
    { icon: <Printer className="w-5 h-5 text-blue-500" />, text: "3D STL Files (Pedal Body)" },
    { icon: <FileCode className="w-5 h-5 text-blue-500" />, text: "Source Code (.ino)" },
    { icon: <Map className="w-5 h-5 text-blue-500" />, text: "Electronic Connection Diagrams" },
    { icon: <Shield className="w-5 h-5 text-blue-500" />, text: "4K High-Quality Assembly Guide" },
    { icon: <Download className="w-5 h-5 text-blue-500" />, text: "Calibration Software" },
    { icon: <HelpCircle className="w-5 h-5 text-blue-500" />, text: "Lifetime Community Support" },
  ];

  return (
    <section id="purchase" className="py-24 px-4 bg-neutral-50 relative overflow-hidden shadow-[0_15px_30px_-15px_rgba(0,0,0,0.05)] z-10">
      <div className="max-w-6xl mx-auto flex flex-col lg:flex-row items-center gap-16">
        
        {/* Left: Product Info */}
        <div className="flex-1 space-y-8">
          <h2 className="text-4xl md:text-5xl font-black text-neutral-900 leading-tight">
            Get the <span className="text-blue-600">Project Files</span>
          </h2>
          <p className="text-xl text-neutral-500 leading-relaxed">
            Download 3D printing STL files, source code (Firmware), and complete electronic schematics to build your own wireless pedal at home.
          </p>

          <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
            {features.map((feature, i) => (
              <div key={i} className="flex items-center gap-3 p-4 bg-neutral-50 rounded-2xl border border-neutral-100 hover:border-blue-200 transition-colors">
                {feature.icon}
                <span className="text-neutral-700 font-medium text-sm">{feature.text}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Right: Pricing Card */}
        <div className="w-full lg:w-[400px] shrink-0">
          <div className="bg-white p-10 rounded-[40px] text-neutral-900 shadow-2xl relative overflow-hidden group border border-neutral-100">
            <div className="absolute top-0 right-0 w-32 h-32 bg-blue-600/5 blur-3xl -mr-16 -mt-16 group-hover:bg-blue-600/10 transition-colors" />
            
            <div className="relative z-10 space-y-8 text-center">
              <div className="inline-block px-4 py-1 bg-blue-600 text-white text-xs font-bold uppercase tracking-widest rounded-full">
                All-in-One Package
              </div>
              
              <div>
                <p className="text-neutral-400 text-sm mb-2 font-medium uppercase tracking-tighter">Total Project Access</p>
                <div className="flex items-center justify-center gap-2 text-neutral-900">
                  <span className="text-2xl font-bold opacity-30 font-mono">$</span>
                  <span className="text-7xl font-black tracking-tighter">45</span>
                  <span className="text-lg font-bold text-neutral-400">USD</span>
                </div>
              </div>

              <ul className="space-y-4 text-sm text-neutral-600 font-medium">
                <li className="flex items-center gap-2 justify-center">
                  <ArrowRight className="w-4 h-4 text-blue-500" /> Instant Download
                </li>
                <li className="flex items-center gap-2 justify-center">
                  <ArrowRight className="w-4 h-4 text-blue-500" /> Secure Payment (PayPal)
                </li>
              </ul>

              <button className="w-full py-5 bg-blue-600 hover:bg-blue-700 text-white text-lg font-bold rounded-2xl transition-all shadow-xl shadow-blue-500/20 active:scale-95 flex items-center justify-center gap-2">
                Buy Files
                <Download className="w-5 h-5" />
              </button>

              <p className="text-[10px] text-neutral-500 font-medium px-4">
                This project is provided "as is" for DIY purposes. Safety first when handling industrial welding equipment.
              </p>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
