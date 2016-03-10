#include "stdafx.h"

uint8 FormatToLength( EFormat eFormat )
{
	if( eFormat <= EFormat::EFormatR32G32B32A32SInt ) return 16;
	if( eFormat <= EFormat::EFormatR32G32B32SInt ) return 12;
	if( eFormat <= EFormat::EFormatX32TypelessG8X24UInt ) return 8;
	if( eFormat <= EFormat::EFormatX24TypelessG8UInt ) return 4;
	if( eFormat <= EFormat::EFormatR16SInt ) return 2;
	if( eFormat <= EFormat::EFormatA8UNorm ) return 1;
	return 0;
}