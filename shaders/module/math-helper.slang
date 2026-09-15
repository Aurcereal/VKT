module math_helper;

public float4x4 rotY(float o) {
    return float4x4(
        cos(o), 0., sin(o), 0.,
        0., 1., 0., 0.,
        -sin(o), 0., cos(o), 0.,
        0., 0., 0., 1.
    );
}

public float sqr(float x) {
    return x * x;
}

public void Swap<T>(inout T v1, inout T v2) {
    T temp = v1;
    v1 = v2;
    v2 = temp;
}