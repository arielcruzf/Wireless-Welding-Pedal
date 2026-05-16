import HeroSection from "@/components/HeroSection";
import PurchaseSection from "@/components/PurchaseSection";
import HardwareSpecs from "@/components/HardwareSpecs";
import CommunitySupport from "@/components/CommunitySupport";
import Footer from "@/components/Footer";

export default function Home() {
  return (
    <main className="w-full flex flex-col items-center overflow-x-hidden pt-0! sm:pt-0!">
      <HeroSection />
      
      {/* Container for subsequent sections to handle z-index correctly with the video background behind */}
      <div className="w-full z-20 relative bg-white pb-0">
        <PurchaseSection />
        <HardwareSpecs />
        <CommunitySupport />
        <Footer />
      </div>
    </main>
  );
}
