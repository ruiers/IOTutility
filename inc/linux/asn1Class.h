enum asn1Tags
{
    ASN1_ARRAY = 0x81,
    ASN1_STRUCT,
    ASN1_BOOLEAN,
    ASN1_BIT_STRING,
    ASN1_INTEGER,
    ASN1_UNSIGNED,
    ASN1_FLOAT,
    ASN1_OCT_STRING,
    ASN1_STRING,
    ASN1_TIME,
    ASN1_BCD,
    ASN1_BOOL_ARRAY
};

typedef struct _ASN1Item
{
    unsigned char tag;
    unsigned char len;
    unsigned char val[0];
} ASN1Item;

typedef struct _ASN1Array
{
    unsigned char tag;
    unsigned char len;
    unsigned char val;
    ASN1Item Items[0];
} ASN1Array;

typedef struct ASN1Bool
{
    unsigned char tag;
    unsigned char len;
    unsigned char val;
} ASN1Bool;

typedef struct ASN1Int
{
    unsigned char tag;
    unsigned char len;
    int           val;
} ASN1Int;

typedef struct _ASN1Float
{
    unsigned char tag;
    unsigned char len;
    float         val;
} ASN1Float;

#define ASN1Size(ASN1Item)   (ASN1Item->len + 2)
#define ASN1Length(ASN1Item) (ASN1Item->len)

ASN1Item* asn1EncodeBool(unsigned char boolValue);
ASN1Item* asn1EncodeInteger(int intValue);
ASN1Item* asn1EncodeFloat(float floatValue);
ASN1Item* asn1EncodeString(char *strSource);
ASN1Array* asn1ArrayCreate();
