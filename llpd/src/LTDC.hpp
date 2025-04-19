#include "LLPD.hpp"

void LLPD::ltdc_init (const unsigned int hSyncWidth, const unsigned int hBackPorch, const unsigned int hFrontPorch, const unsigned int hWidth,
			const unsigned int vSyncHeight, const unsigned int vBackPorch, const unsigned int vFrontPorch, const unsigned int vHeight,
			const LTDC_HSYNC_POL& hSyncPol, const LTDC_VSYNC_POL& vSyncPol, const LTDC_DE_POL& dePol, const LTDC_PCLK_POL& pClkPol,
			const uint8_t bgR, const uint8_t bgG, const uint8_t bgB)
{
	tempRegVal = LTDC->GCR;

	// set global control (ignoring dither for now, but may be interesting to explore in the future)
	tempRegVal |= ( static_cast<uint32_t>(pClkPol) << LTDC_GCR_PCPOL_Pos )
			| ( static_cast<uint32_t>(vSyncPol) << LTDC_GCR_VSPOL_Pos )
			| ( static_cast<uint32_t>(hSyncPol) << LTDC_GCR_HSPOL_Pos );
	LTDC->GCR = tempRegVal;

	// set synchronization size
	tempRegVal = 0;
	tempRegVal |= ( (hSyncWidth - 1) << LTDC_SSCR_HSW_Pos ) | ( (vSyncHeight - 1) << LTDC_SSCR_VSH_Pos );
	LTDC->SSCR = tempRegVal;

	// set back porch
	tempRegVal = 0;
	tempRegVal |= ( (hSyncWidth + hBackPorch - 1) << LTDC_BPCR_AHBP_Pos ) | ( (vSyncHeight + vBackPorch - 1) << LTDC_BPCR_AVBP_Pos );
	LTDC->BPCR = tempRegVal;

	// set active width/height
	tempRegVal = 0;
	tempRegVal |= ( (hSyncWidth + hBackPorch + hWidth - 1) << LTDC_AWCR_AAW_Pos )
			| ( (vSyncHeight + vBackPorch + vHeight - 1) << LTDC_AWCR_AAH_Pos );
	LTDC->AWCR = tempRegVal;

	// set total width/height
	tempRegVal = 0;
	tempRegVal |= ( (hSyncWidth + hBackPorch + hWidth + hFrontPorch - 1) << LTDC_TWCR_TOTALW_Pos )
			| ( (vSyncHeight + vBackPorch + vHeight + vFrontPorch - 1) << LTDC_TWCR_TOTALH_Pos );
	LTDC->TWCR = tempRegVal;

	// set background color
	tempRegVal = 0;
	tempRegVal |= ( bgR << LTDC_BCCR_BCRED_Pos ) | ( bgG << LTDC_BCCR_BCGREEN_Pos ) | ( bgB << LTDC_BCCR_BCBLUE_Pos );
	LTDC->BCCR = tempRegVal;

	// enable ltdc
	LTDC->GCR |= LTDC_GCR_LTDCEN;
}

void LLPD::ltdc_layer_init (const LTDC_LAYER& layer, const unsigned int hStart, const unsigned int hStop, const unsigned int vStart,
				const unsigned int vStop, const LTDC_PIXEL_FORMAT& pixelFormat, const uint8_t alphaBlendingConst,
				const uint8_t defaultAlphaVal, const LTDC_BLEND_FACTOR1& blendFactor1, const LTDC_BLEND_FACTOR2& blendFactor2,
				const uint32_t fbStartAddress, const uint32_t fbLineLength, const uint32_t fbNumLines, const uint8_t bgR,
				const uint8_t bgG, const uint8_t bgB)
{
	LTDC_Layer_TypeDef* layerPtr = nullptr;
	if ( layer == LTDC_LAYER::LAYER_1 )
	{
		layerPtr = LTDC_Layer1;
	}
	else if ( layer == LTDC_LAYER::LAYER_2 )
	{
		layerPtr = LTDC_Layer2;
	}

	uint32_t tempRegVal = 0;

	// set window horizontal start/stop position
	tempRegVal |= ( (hStart + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> LTDC_BPCR_AHBP_Pos) + 1) << LTDC_LxWHPCR_WHSTPOS_Pos )
			|  ( (hStart + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> LTDC_BPCR_AHBP_Pos) + 1 + hStop) << LTDC_LxWHPCR_WHSPPOS_Pos );
	layerPtr->WHPCR = tempRegVal;

	// set window vertical start/stop position
	tempRegVal = 0;
	tempRegVal |= ( (vStart + ((LTDC->BPCR & LTDC_BPCR_AVBP) >> LTDC_BPCR_AVBP_Pos) + 1) << LTDC_LxWVPCR_WVSTPOS_Pos )
			|  ( (vStart + ((LTDC->BPCR & LTDC_BPCR_AVBP) >> LTDC_BPCR_AVBP_Pos) + 1 + vStop) << LTDC_LxWVPCR_WVSPPOS_Pos );
	layerPtr->WVPCR = tempRegVal;

	// set pixel format
	layerPtr->PFCR = static_cast<uint32_t>( pixelFormat );

	// set default color
	tempRegVal = 0;
	tempRegVal |= ( bgR << LTDC_LxDCCR_DCRED_Pos ) | ( bgG << LTDC_LxDCCR_DCGREEN_Pos ) << ( bgB << LTDC_LxDCCR_DCBLUE_Pos )
			| ( defaultAlphaVal << LTDC_LxDCCR_DCALPHA_Pos );
	layerPtr->DCCR = tempRegVal;

	// set constant alpha value
	tempRegVal = 0;
	tempRegVal |= ( alphaBlendingConst << LTDC_LxCACR_CONSTA_Pos );
	layerPtr->CACR = tempRegVal;

	// set blending factor
	tempRegVal = 0;
	tempRegVal |= ( static_cast<uint32_t>(blendFactor1) << LTDC_LxBFCR_BF1_Pos )
			| ( static_cast<uint32_t>(blendFactor2) << LTDC_LxBFCR_BF2_Pos );
	layerPtr->BFCR = tempRegVal;

	// set frame buffer start address
	layerPtr->CFBAR = fbStartAddress;

	// set frame buffer line length
	uint8_t bytesPerPixel = 0;
	if ( pixelFormat == LTDC_PIXEL_FORMAT::ARGB8888 )
	{
		bytesPerPixel = 4;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::RGB888 )
	{
		bytesPerPixel = 3;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::RGB565 )
	{
		bytesPerPixel = 2;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::ARGB1555 )
	{
		bytesPerPixel = 2;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::ARGB4444 )
	{
		bytesPerPixel = 2;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::L8 )
	{
		bytesPerPixel = 1;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::AL44 )
	{
		bytesPerPixel = 1;
	}
	else if ( pixelFormat == LTDC_PIXEL_FORMAT::AL88 )
	{
		bytesPerPixel = 2;
	}
	tempRegVal = 0;
	tempRegVal |= ( ((fbLineLength * bytesPerPixel) + 7) << LTDC_LxCFBLR_CFBLL_Pos )
			| ( (fbLineLength * bytesPerPixel) << LTDC_LxCFBLR_CFBP_Pos );
	layerPtr->CFBLR = tempRegVal;

	// set frame buffer line number
	tempRegVal = 0;
	tempRegVal |= ( fbNumLines << LTDC_LxCFBLNR_CFBLNBR_Pos );
	layerPtr->CFBLNR = tempRegVal;
}

void LLPD::ltdc_start()
{
	// set immediate reload in SRCR
	LTDC->SRCR |= LTDC_SRCR_IMR;
}

void LLPD::ltdc_layer_enable (const LTDC_LAYER& layer)
{
	LTDC_Layer_TypeDef* layerPtr = nullptr;
	if ( layer == LTDC_LAYER::LAYER_1 )
	{
		layerPtr = LTDC_Layer1;
	}
	else if ( layer == LTDC_LAYER::LAYER_2 )
	{
		layerPtr = LTDC_Layer2;
	}

	// enable layer
	layerPtr->CR |= LTDC_LxCR_LEN;
}

void LLPD::ltdc_layer_disable (const LTDC_LAYER& layer)
{
	LTDC_Layer_TypeDef* layerPtr = nullptr;
	if ( layer == LTDC_LAYER::LAYER_1 )
	{
		layerPtr = LTDC_Layer1;
	}
	else if ( layer == LTDC_LAYER::LAYER_2 )
	{
		layerPtr = LTDC_Layer2;
	}

	// disable layer
	layerPtr->CR &= ~(LTDC_LxCR_LEN);
}
