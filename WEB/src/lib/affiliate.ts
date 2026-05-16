/**
 * Affiliate Link Utility
 * 
 * Simulates tracking ID generation for components.
 * Default ID: WELDER-DIY-2026
 */

export const DEFAULT_TRACKING_ID = "WELDER-DIY-2026";

export function generateAffiliateLink(url: string, trackingId: string = DEFAULT_TRACKING_ID): string {
  try {
    const urlObj = new URL(url);
    // AliExpress specific simulation (adding tracking parameters)
    urlObj.searchParams.set("aff_platform", "link-health");
    urlObj.searchParams.set("aff_trace_key", trackingId);
    urlObj.searchParams.set("sk", "pedal_init"); // Static key for stable hydration
    return urlObj.toString();
  } catch (e) {
    return url; // Fallback to original URL if invalid
  }
}
