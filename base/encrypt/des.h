// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#ifndef yxDESH
#define yxDESH
#include <string>
// support CBC ECB mode,you can  padding  PKCS5 or NOPKCS by yuzj 2010 5 10
#define NOPKCS 0  //默认
#define PKCS5 1

#define ECB_MODE 0  //默认
#define CBC_MODE 1

class yxDES {
 public:
  static std::string Encrypt(const std::string& src, const char* key,
                             int iMode = ECB_MODE, int iPKCS = PKCS5);
  static std::string Decrypt(const std::string& src, const char* key,
                             int iMode = ECB_MODE, int iPKCS = PKCS5);

 private:
  yxDES(int length = 8192);
  ~yxDES();

  void SetModeAndPKCS(int iMode = 0, int iPKCS = 0);
  void SetIV(char* srcBytes);

  void InitializeKey(const char* srcBytes, unsigned int keyN);

  void EncryptData(char* _srcBytes, unsigned int keyN);
  void DecryptData(char* _srcBytes, unsigned int keyN);

  void EncryptAnyLength(char* _srcBytes, unsigned int _bytesLength,
                        unsigned int keyN);
  void DecryptAnyLength(char* _srcBytes, unsigned int _bytesLength,
                        unsigned int keyN);

  void Bytes2Bits(const char* srcBytes, char* dstBits, unsigned int sizeBits);
  void Bits2Bytes(char* dstBytes, char* srcBits, unsigned int sizeBits);

  void Int2Bits(unsigned int srcByte, char* dstBits);

  void Bits2Hex(char* dstHex, char* srcBits, unsigned int sizeBits);
  void Hex2Bits(char* srcHex, char* dstBits, unsigned int sizeBits);

  char* GetCiphertextInBinary();
  char* GetCiphertextInHex();
  char* GetCiphertextInBytes();

  char* GetPlaintext();
  char* GetCiphertextAnyLength();
  char* GetPlaintextAnyLength();

  void ConvertCiphertext2Hex(char* szPlainInBytes);
  int ConvertHex2Ciphertext(const char* szCipherInBytes);

  char* GetCiphertextData();
  char* GetHexCipherAnyLengthData();

 private:
  int m_iLength;
  char szSubKeys[2][16][48];    //储存2个16组48位密钥,第2个用于3DES
  char szCiphertextRaw[64];     //储存二进制密文(64个Bits) int 0,1
  char szPlaintextRaw[64];      //储存二进制密文(64个Bits) int 0,1
  char szCiphertextInBytes[8];  //储存8位密文
  char szPlaintextInBytes[8];   //储存8位明文字符串

  char szCiphertextInBinary[65];  //储存二进制密文(64个Bits) char
                                  //'0','1',最后一位存'\0'
  char szCiphertextInHex[17];     //储存十六进制密文,最后一位存'\0'
  char szPlaintext[9];  //储存8位明文字符串,最后一位存'\0'

  char* szFCiphertextAnyLength;  //任意长度密文
  char* szFPlaintextAnyLength;   //任意长度明文字符串

  char* szCiphertextData;
  char* hexCiphertextAnyLength;

  char sziv[8];     //加密IV
  char szivRev[8];  //解密IV

  int m_iMode;  //加解密模式
  int m_iPkcs;  //填充模式

  int data_base_length_;  //用于编解码需要的字符缓存长度

  void CreateSubKey(char* sz_56key, unsigned int keyN);

  void FunctionF(char* sz_Li, char* sz_Ri, unsigned int iKey,
                 unsigned int keyN);

  void InitialPermuteData(char* _src, char* _dst);

  void ExpansionR(char* _src, char* _dst);
  void XOR(char* szParam1, char* szParam2, unsigned int uiParamLength,
           char* szReturnValueBuffer);
  void CompressFuncS(char* _src48, char* _dst32);
  void PermutationP(char* _src, char* _dst);
};

#endif
