#include <stdlib.h>
#include <string.h>
#include "asn1Class.h"

ASN1Item* asn1EncodeBool(unsigned char boolValue)
{
    ASN1Item* asn1BoolItem = malloc(sizeof(unsigned char) + 2);

    asn1BoolItem->tag = ASN1_INTEGER;
    asn1BoolItem->len = sizeof(unsigned char);
    asn1BoolItem->val[0] = boolValue;

    return asn1BoolItem;
}

ASN1Item* asn1EncodeInteger(int intValue)
{
    ASN1Item* asn1IntItem = malloc(sizeof(int) + 2);

    asn1IntItem->tag = ASN1_INTEGER;
    asn1IntItem->len = sizeof(int);
    *((int *) asn1IntItem->val) = intValue;

    return asn1IntItem;
}

ASN1Item* asn1EncodeFloat(float floatValue)
{
    ASN1Item* asn1FloatItem = malloc(sizeof(float) + 2);

    asn1FloatItem->tag = ASN1_INTEGER;
    asn1FloatItem->len = sizeof(float);
    *((float *) asn1FloatItem->val) = floatValue;

    return asn1FloatItem;
}

ASN1Item* asn1EncodeString(char *strSource)
{
    ASN1Item* asn1StrItem = malloc(strlen(strSource) + 2);

    asn1StrItem->tag = ASN1_STRING;
    asn1StrItem->len = strlen(strSource);
    memcpy(asn1StrItem->val, strSource, strlen(strSource));

    return asn1StrItem;
}

ASN1Array* asn1ArrayCreate()
{
    ASN1Array* asn1Array = malloc(sizeof(unsigned char) + 2);

    asn1Array->tag = ASN1_INTEGER;
    asn1Array->len = sizeof(unsigned char);
    asn1Array->val = 0;

    return asn1Array;
}
