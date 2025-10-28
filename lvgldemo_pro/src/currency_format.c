#include <stdlib.h>
#include <stdio.h>
#include "libapi_xpos/inc/mfsdk_sys.h"
#include "libapi_xpos/inc/mfsdk_gui.h"
#include "tracedef.h"
#include "currency_format.h"

/*
    default:2
*/
static unsigned int s_CurrencyExponent = 2;
static unsigned char s_ThousandSep = APP_THOUSAND_SEPARATOR_NULL;
static unsigned char s_DecimalMark = APP_DECIMAL_MARK_DOT;

/**
 * @brief
 * 
 * @param[] unsigned char thousandSep
 */
void AppSetThousandSep(unsigned char thousandSep)
{
	if(thousandSep == APP_THOUSAND_SEPARATOR_NULL \
		|| APP_THOUSAND_SEPARATOR_SPACE == thousandSep \
		|| APP_THOUSAND_SEPARATOR_COMMA == thousandSep \
		|| APP_THOUSAND_SEPARATOR_DOT == thousandSep)
	{
		s_ThousandSep = thousandSep;
	}
}
/**
 * @brief
 * 
 * @return 
 */
unsigned char AppGetThousandSep(void)
{
	return s_ThousandSep;
}
/**
 * @brief
 * 
 * @param[] unsigned char decimalMark
 */
void AppSetDecimalMark(unsigned char decimalMark)
{
	if(decimalMark == APP_DECIMAL_MARK_NULL \
		|| APP_DECIMAL_MARK_DOT == decimalMark \
		|| APP_DECIMAL_MARK_COMMA == decimalMark)
	{
		s_DecimalMark = decimalMark;
	}
}
/**
 * @brief
 * 
 * @return 
 */
unsigned char AppGetDecimalMark(void)
{
	return s_DecimalMark;
}

/**
 * @brief Currency Exponent
 *
 * @param[in] unsigned int exponent
 * eg. USD Currency Exponent is 2 ->1.00
 * eg. IDR Currency Exponent is 0 ->1
 */
void AppSetCurrencyExponent(unsigned int exponent)
{
    if (exponent >= 0 && exponent < 5)
    {
        s_CurrencyExponent = exponent;
    }
}

unsigned int AppGetCurrencyExponent(void)
{
    return s_CurrencyExponent;
}


/**
 * @brief Exponential operations
 *
 * @param[in] unsigned int iIndex
 * @param[in] unsigned int iPower
 * @return value
 */
unsigned int AppPow(unsigned int iIndex, unsigned int iPower)
{
    unsigned int iTmep = 1;
    unsigned int i = 0;

    for (i = 1; i <= iPower; i++) 
	{
        iTmep = iIndex * iTmep;
    }

    return iTmep;
}

/**
 * @brief
 *
 * @param[in] char* str  str
 * @param[in] char charToRemove
 * @return dest point
 */
char* AppRemoveSymbol(char *str, char charToRemove) 
{
    char *src = str, *dst = str , *pstr = str;
	
    while (*src != '\0') 
	{
        if (*src != charToRemove) 
		{
            *dst = *src;
            dst++;
        }
        src++;
    }
    *dst = '\0';
	APP_TRACE("AppRemoveSymbol:%s\r\n", pstr);
	return pstr;
}
/**
 * @brief
 * 
 * @param[in] char* str
 * @param[in] char oldChar
 * @param[in] char newChar
 * @return dest point
 */
char* AppReplaceSymbol(char* str, char oldChar , char newChar)
{
	char *pstr = str;
	
	while (*pstr != '\0') 
	{
        if (*pstr == oldChar) 
		{
            *pstr = newChar;
        }
        pstr++;
    }

	APP_TRACE("AppReplaceSymbol:%s\r\n", str);
	return str;
}

/**
 * @brief
 *
 * @param[in] char* str  dest
 * @param[in] char separator  thousands separator THOUSAND_SEPARATOR_SPACE/THOUSAND_SEPARATOR_DOT/THOUSAND_SEPARATOR_COMMA
 * @return dest point
 */
char* AppAddSeparator(char* str, char separator)
{
    int len = strlen(str);
    int separatorCount = (len - 1) / 3;
    int newLen = len + separatorCount;
    char result[128] = { 0 };

	if(APP_THOUSAND_SEPARATOR_SPACE != separator \
		&& APP_THOUSAND_SEPARATOR_DOT != separator \
		&& APP_THOUSAND_SEPARATOR_COMMA != separator )
	{
		return str;
	}

    int i = len - 1;
    int j = newLen - 1;
    int count = 0;

    while (i >= 0)
    {
        if (count == 3)
        {
            result[j--] = separator;
            count = 0;
        }
        result[j--] = str[i--];
        count++;
    }

    for (i = 0; i < newLen; i++)
    {
        str[i] = result[i];
    }
    str[i] = '\0';
    APP_TRACE("AppAddSeparator:%s\r\n", str);
    return str;
}

/**
 * @brief
 *
 * @param[out] unsigned char* dest
 * @param[in] long long amt
 * @param[in] char separator  thousands separator : THOUSAND_SEPARATOR_SPACE/THOUSAND_SEPARATOR_DOT/THOUSAND_SEPARATOR_COMMA
 * @param[in] char decimalMark
 * @return dest point
 */
unsigned char* AppFormatAmount(unsigned char* dest, long long amt, char separator, char decimalMark)
{
    char format[32] = { 0 };
    char temp[128] = { 0 };
    int powValue = AppPow(10, AppGetCurrencyExponent());

    if (AppGetCurrencyExponent() == 0)
    {
        sprintf((char*)dest, "%lld", amt);
        AppAddSeparator((char*)dest, separator);
    }
    else
    {
        sprintf(temp, "%lld", amt / powValue);
        AppAddSeparator(temp, separator);

		if(decimalMark == APP_DECIMAL_MARK_DOT || APP_DECIMAL_MARK_COMMA == decimalMark)
		{
        	sprintf(format, "%c%%0%dlld", decimalMark, AppGetCurrencyExponent());
		}
		else
		{
			sprintf(format, "%c%%0%dlld", APP_DECIMAL_MARK_DOT, AppGetCurrencyExponent());
		}
		
        sprintf(temp + strlen(temp), format, amt % powValue);
        sprintf((char*)dest, "%s", temp);
    }
    APP_TRACE("FormatAmount:%s\r\n", dest);
    return dest;
}

/**
 * @brief  IDR
 * 
 * @param[in] unsigned char* dest
 * @param[in] long long amt
 * @return dest point
 */
unsigned char* FormatAmountCurrencyExponent0(unsigned char* dest, long long amt)
{
    AppSetCurrencyExponent(0);
    return AppFormatAmount(dest, amt, APP_THOUSAND_SEPARATOR_DOT, APP_DECIMAL_MARK_NULL);
}
/**
 * @brief
 * 
 * @param[in] unsigned char* dest
 * @param[in] long long amt
 * @return dest point
 */
unsigned char* FormatAmountCNY(unsigned char* dest, long long amt)
{
	AppSetCurrencyExponent(2);

	return AppFormatAmount(dest, amt, APP_THOUSAND_SEPARATOR_NULL, APP_DECIMAL_MARK_DOT);
}


/**
 * @brief
 * 
 * @param[in] unsigned char* dest
 * @param[in] long long amt
 * @return dest point
 */
unsigned char* AppFormatAmountFinal(unsigned char* dest, long long amt)
{
	return AppFormatAmount(dest, amt, AppGetThousandSep(), AppGetDecimalMark());
}


/**
 * @brief
 * 
 * @param[in] long long amt
 */
void AppFormatSegmentLedAmount(long long amt)
{
	int numShowLen = 6; 
	unsigned char dispAmt[32] = {0};

	numShowLen = (MfSdkSysGetHardwareVer() >= 500) ? 7 : 6;
	
	AppFormatAmount(dispAmt, amt, AppGetThousandSep(), AppGetDecimalMark());
	
	MfSdkGuiLedAmount(dispAmt);
}

