#pragma once

enum EAttributeModifyType	//属性修改类型，最终值=(Base*Mul1+Add1)*Mul2+Add2
{
	eAttrModifyType_Base,
	eAttrModifyType_Mul1,
	eAttrModifyType_Add1,
	eAttrModifyType_Mul2,
	eAttrModifyType_Add2,

	eAttrModifyType_Count
};

struct SAttribute
{
	SAttribute( int32 base = 0 ) : base( base ), mul1( 0 ), add1( 0 ), mul2( 0 ), add2( 0 ) {}

	operator int32 () const {
		return ( base * ( mul1 + 10000LL ) / 10000 + add1 ) * ( mul2 + 10000 ) / 10000 + add2;
	}

	int32 GetMaxValue()
	{
		return ( base * ( mul1 + 10000LL ) / 10000 + add1 ) * ( mul2 + 10000 ) / 10000;
	}
	void SetCurValue( int32 value )
	{
		add2 = Min( Max( value, 0 ) - GetMaxValue(), 0 );
	}
	void ModifyCurValue( int32 value )
	{
		add2 += value;
		if( add2 > 0 )
			add2 = 0;
		int32 nMax = GetMaxValue();
		if( add2 < -nMax )
			add2 = -nMax;
	}

	union
	{
		struct
		{
			int32 base;
			int32 mul1;
			int32 add1;
			int32 mul2;
			int32 add2;
		};
		int32 nValue[eAttrModifyType_Count];
	};
};
