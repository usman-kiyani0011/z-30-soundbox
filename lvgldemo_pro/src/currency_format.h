/**
 * @file currency_format.h
 * @author CHAR
 * @brief 
 * @date 2024-5-22
 * @copyright Fujian MoreFun Electronic Technology Co., Ltd.
 */
#ifndef __CURRENCY_FORMAT_H__
#define __CURRENCY_FORMAT_H__

#define APP_THOUSAND_SEPARATOR_NULL (0xFF)
#define APP_THOUSAND_SEPARATOR_SPACE ' '
#define APP_THOUSAND_SEPARATOR_COMMA ','
#define APP_THOUSAND_SEPARATOR_DOT '.'

#define APP_DECIMAL_MARK_NULL (0xFF)
#define APP_DECIMAL_MARK_DOT '.'
#define APP_DECIMAL_MARK_COMMA ','

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
/**
 * @brief
 *
 * @param[in] char* str  dest
 * @param[in] char separator  thousands separator THOUSAND_SEPARATOR_SPACE/THOUSAND_SEPARATOR_DOT/THOUSAND_SEPARATOR_COMMA
 * @return dest point
 */
LIB_EXPORT char* AppAddSeparator(char* str, char separator);
/**
 * @brief
 *
 * @param[out] unsigned char* dest
 * @param[in] long long amt
 * @param[in] char separator  thousands separator : THOUSAND_SEPARATOR_SPACE/THOUSAND_SEPARATOR_DOT/THOUSAND_SEPARATOR_COMMA
 * @param[in] char decimalMark
 * @return dest point
 */
LIB_EXPORT unsigned char* AppFormatAmount(unsigned char* dest, long long amt, char separator, char decimalMark);
LIB_EXPORT unsigned int AppGetCurrencyExponent(void);

/**
 * @brief Exponential operations
 *
 * @param[in] unsigned int iIndex
 * @param[in] unsigned int iPower
 * @return value
 */
LIB_EXPORT unsigned int AppPow(unsigned int iIndex, unsigned int iPower);
/**
 * @brief
 *
 * @param[in] char* str  dest
 * @param[in] char data
 * @return dest point
 */
LIB_EXPORT char* AppRemoveSymbol(char* str, char data);
/**
 * @brief Currency Exponent
 *
 * @param[in] unsigned int exponent
 * eg. USD Currency Exponent is 2 ->1.00
 * eg. IDR Currency Exponent is 0 ->1
 */
LIB_EXPORT void AppSetCurrencyExponent(unsigned int exponent);
/**
 * @brief  IDR
 * 
 * @param[in] unsigned char* dest
 * @param[in] long long amt
 * @return dest point
 */
LIB_EXPORT unsigned char* FormatAmountCurrencyExponent0(unsigned char* dest, long long amt);
/**
 * @brief
 * 
 * @param[in] unsigned char* dest
 * @param[in] long long amt
 * @return dest point
 */
LIB_EXPORT unsigned char* AppFormatAmountFinal(unsigned char* dest, long long amt);


LIB_EXPORT void AppFormatSegmentLedAmount(long long amt);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CURRENCY_FORMAT_H__ */
