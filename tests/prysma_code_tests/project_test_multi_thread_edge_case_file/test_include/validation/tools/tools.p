include "comparison.p";

fn int32 absoluteValue(arg int32 n)
{
    if (n < 0) {
        dec int32 neg = 0 - n;
        return neg;
    }
    return n;
}

fn int32 maximum(arg int32 a, arg int32 b)
{
    dec bool aIsGreater = call isGreaterInt(a, b);
    if (aIsGreater == true) {
        return a;
    }
    return b;
}

fn int32 minimum(arg int32 a, arg int32 b)
{
    dec bool aIsLess = call isLessInt(a, b);
    if (aIsLess == true) {
        return a;
    }
    return b;
}
