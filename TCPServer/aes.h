///////////////////////////////////////////////////////////////////////////////
// 文 件 名：AES.h
// 描    述：AES加密算法
// 创 建 人：Liangbofu
// 创建日期：2009-07-17
///////////////////////////////////////////////////////////////////////////////
#ifndef __AES_H
#define __AES_H

#ifdef __cplusplus
    extern "C" {
#endif

// 以bit为单位的密钥长度，只能为 128，192 和 256 三种
#define AES_KEY_LENGTH	128

// 加解密模式
#define AES_MODE_ECB	0				// 电子密码本模式（一般模式）
#define AES_MODE_CBC	1				// 密码分组链接模式
#define AES_MODE		AES_MODE_CBC
#define IV "1234567890abcdef"


///////////////////////////////////////////////////////////////////////////////
//	函数名：	AES_Init
//	描述：		初始化，在此执行扩展密钥操作。
//	输入参数：	pKey -- 原始密钥，其长度必须为 AES_KEY_LENGTH/8 字节。
//	输出参数：	无。
//	返回值：	无。
///////////////////////////////////////////////////////////////////////////////
void AES_Init(const void *pKey);

//////////////////////////////////////////////////////////////////////////
//	函数名：	AES_Encrypt
//	描述：		加密数据
//	输入参数：	pPlainText	-- 明文，即需加密的数据，其长度为nDataLen字节。
//				nDataLen	-- 数据长度，以字节为单位，必须为AES_KEY_LENGTH/8的整倍数。
//				pIV			-- 初始化向量，如果使用ECB模式，可设为NULL。
//	输出参数：	pCipherText	-- 密文，即由明文加密后的数据，可以与pPlainText相同。
//	返回值：	无。
//////////////////////////////////////////////////////////////////////////
void AES_Encrypt(const unsigned char *pPlainText, unsigned char *pCipherText,
                 unsigned int nDataLen, const unsigned char *pIV);

//////////////////////////////////////////////////////////////////////////
//	函数名：	AES_Decrypt
//	描述：		解密数据
//	输入参数：	pCipherText -- 密文，即需解密的数据，其长度为nDataLen字节。
//				nDataLen	-- 数据长度，以字节为单位，必须为AES_KEY_LENGTH/8的整倍数。
//				pIV			-- 初始化向量，如果使用ECB模式，可设为NULL。
//	输出参数：	pPlainText  -- 明文，即由密文解密后的数据，可以与pCipherText相同。
//	返回值：	无。
//////////////////////////////////////////////////////////////////////////
void AES_Decrypt(unsigned char *pPlainText, const unsigned char *pCipherText,
                 unsigned int nDataLen, const unsigned char *pIV);

//////////////////////////////////////////////////////////////////////////
//	函数名：	computeEncryptedSize
//	描述：		计算加密后的字符串长度，使其长度为AES_KEY_LENGTH/8的整数倍
//	输入参数：	in_len -- 加密前字符串长度
//	返回值：	out_len -- 字符串扩展后的长度，必须为AES_KEY_LENGTH/8的整倍数。
//////////////////////////////////////////////////////////////////////////
int computeEncryptedSize(int in_len);

//////////////////////////////////////////////////////////////////////////
//	函数名：	expandText
//	描述：		加密字符串
//	输入参数：	input -- 输入字符串
//	输入参数：	in_len -- 输入字符串的长度
//	输入参数：  out_len -- 输出字符串的长度
//	输出参数：	output -- 输出字符串，在input的后面补out_len-in_len个'\0'
//////////////////////////////////////////////////////////////////////////
void expandText(const unsigned char *input, unsigned char *output, int in_len, int out_len);

//////////////////////////////////////////////////////////////////////////
//	函数名：	encrypt
//	描述：		加密字符串
//	输入参数：	str -- 字符串，原址加密
//	输入参数：	len -- 字符串的长度
//	输入参数：  key -- 密钥
//////////////////////////////////////////////////////////////////////////
void Encrypt(unsigned char *str, const unsigned char *key, int len);

//////////////////////////////////////////////////////////////////////////
//	函数名：	decrypt
//	描述：		解密字符串
//	输入参数：	str -- 字符串，原址解密
//	输入参数：  key -- 密钥
//	输入参数：	len -- 字符串长度
//////////////////////////////////////////////////////////////////////////
void Decrypt(unsigned char *str, const unsigned char *key, int len);
#ifdef __cplusplus
    }
#endif


#endif	// __AES_H
