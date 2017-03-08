#pragma once
//-----------------------------------------------------------------------------
//

template < int _bits >
class Blob
{
private:
  uint8_t bytes[(_bits+7)/8];

public:
  Blob()
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] = 0;
    }
  }

  Blob ( int x )
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] = 0;
    }

    *(int*)bytes = x;
  }

  Blob ( const Blob & k )
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] = k.bytes[i];
    }
  }

  Blob & operator = ( const Blob & k )
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] = k.bytes[i];
    }

    return *this;
  }

  Blob ( uint64_t a, uint64_t b )
  {
    uint64_t t[2] = {a,b};
    set(&t,16);
  }

  void set ( const void * blob, size_t len )
  {
    const uint8_t * k = (const uint8_t*)blob;

    len = len > sizeof(bytes) ? sizeof(bytes) : len;

    for(size_t i = 0; i < len; i++)
    {
      bytes[i] = k[i];
    }

    for(size_t i = len; i < sizeof(bytes); i++)
    {
      bytes[i] = 0;
    }
  }

  uint8_t & operator [] ( int i )
  {
    return bytes[i];
  }

  const uint8_t & operator [] ( int i ) const
  {
    return bytes[i];
  }

  //----------
  // boolean operations

  bool operator < ( const Blob & k ) const
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      if(bytes[i] < k.bytes[i]) return true;
      if(bytes[i] > k.bytes[i]) return false;
    }

    return false;
  }

  bool operator == ( const Blob & k ) const
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      if(bytes[i] != k.bytes[i]) return false;
    }

    return true;
  }

  bool operator != ( const Blob & k ) const
  {
    return !(*this == k);
  }

  //----------
  // bitwise operations

  Blob operator ^ ( const Blob & k ) const
  {
    Blob t;

    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      t.bytes[i] = bytes[i] ^ k.bytes[i];
    }

    return t;
  }

  Blob & operator ^= ( const Blob & k )
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] ^= k.bytes[i];
    }

    return *this;
  }

  int operator & ( int x )
  {
    return (*(int*)bytes) & x;
  }

  Blob & operator &= ( const Blob & k )
  {
    for(size_t i = 0; i < sizeof(bytes); i++)
    {
      bytes[i] &= k.bytes[i];
    }
  }

  Blob operator << ( int c )
  {
    Blob t = *this;

    lshift(&t.bytes[0],sizeof(bytes),c);

    return t;
  }

  Blob operator >> ( int c )
  {
    Blob t = *this;

    rshift(&t.bytes[0],sizeof(bytes),c);

    return t;
  }

  Blob & operator <<= ( int c )
  {
    lshift(&bytes[0],sizeof(bytes),c);

    return *this;
  }

  Blob & operator >>= ( int c )
  {
    rshift(&bytes[0],sizeof(bytes),c);

    return *this;
  }

  void print_as_hex () {
    for (int i=0;i<sizeof(bytes);i++)
      printf("%02x",bytes[i]);
    printf("\n"); // nl ok

  }
  int bits () {
    return _bits;
  }
  //----------

};

typedef Blob<128> uint128_t;
typedef Blob<96>  uint96_t;
typedef Blob<256> uint256_t;

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
