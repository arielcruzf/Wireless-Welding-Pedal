import type { Metadata } from "next";
import { Inter, Outfit } from "next/font/google";
import "./globals.css";

const inter = Inter({
  variable: "--font-inter",
  subsets: ["latin"],
});

const outfit = Outfit({
  variable: "--font-outfit",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "Wireless Welder Pedal | Open Source DIY",
  description: "Wireless welding pedal created with 3D printing and generic electronic components. 5-pin connection to your welder.",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className="scroll-smooth">
      <body
        className={`${inter.variable} ${outfit.variable} antialiased min-h-screen flex flex-col pt-16 sm:pt-0`}
      >
        {children}
      </body>
    </html>
  );
}
