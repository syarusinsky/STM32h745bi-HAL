#include "LLPD.hpp"

void LLPD::ltdc_init (const unsigned int hSyncWidth, const unsigned int hBackPorch, const unsigned int hFrontPorch, const unsigned int hWidth,
			const unsigned int vSyncHeight, const unsigned int vBackPorch, const unsigned int vFrontPorch, const unsigned int vHeight,
			const LTDC_HSYNC_POL& hSyncPol, const LTDC_VSYNC_POL& vSyncPol, const LTDC_DE_POL& dePol, const LTDC_PCLK_POL& pClkPol,
			const uint8_t bgR, const uint8_t bgG, const uint8_t bgB)
{
	// set GCR
	// set SSCR
	// set BPCR
	// set AWCR
	// set TWCR
	// set BCCR
	// enable ltdc
}

void LLPD::ltdc_layer_init (const LTDC_LAYER& layer, const unsigned int hStart, const unsigned int hStop, const unsigned int vStart,
				const unsigned int vStop, const LTDC_PIXEL_FORMAT& pixelFormat, const uint8_t alphaBlendingConst,
				const uint8_t defaultAlphaVal, const LTDC_BLEND_FACTOR1& blendFactor1, const LTDC_BLEND_FACTOR2& blendFactor2,
				const uint32_t fbStartAddress, const uint32_t fbLineLength, const uint32_t fbNumLines, const uint8_t bgR,
				const uint8_t bgG, const uint8_t bgB)
{
	// set WHPCR
	// set WVPCR
	// set PFCR
	// set DCCR
	// set CACR
	// set BFCR
	// set CFBAR
	// set CFBLR
	// set CFBLNR
	// enable layer
}

void LLPD::ltdc_start()
{
	// set immediate reload in SRCR
}

void LLPD::ltdc_layer_enable (const LTDC_LAYER& layer)
{
}

void LLPD::ltdc_layer_disable (const LTDC_LAYER& layer)
{
}
