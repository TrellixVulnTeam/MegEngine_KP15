/**
 * \file dnn/src/fallback/conv_bias/gi/utils.h
 * MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2014-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include <cstring>
#include "src/common/utils.h"
#include "src/fallback/general_intrinsic/gi_float.h"

namespace megdnn {
namespace fallback {

template <typename ctype, size_t len>
struct Vector;

template <>
struct Vector<float, 4> {
    GI_FLOAT32_t value;
    Vector() {}
    Vector(const float v) { value = GiBroadcastFloat32(v); }
    Vector(const Vector& lr) { value = lr.value; }
    Vector(const Vector&& lr) { value = std::move(lr.value); }
    Vector(const GI_FLOAT32_t& v) { value = v; }
    static Vector load(const float* addr) {
        Vector v;
        v.value = GiLoadFloat32(addr);
        return v;
    }
    static void save(float* addr, const Vector& v) { GiStoreFloat32(addr, v.value); }
    void save(float* addr) { save(addr, *this); }
    Vector operator+(const Vector& lr) {
        Vector dst;
        dst.value = GiAddFloat32(value, lr.value);
        return dst;
    }
    Vector& operator+=(const Vector& lr) {
        value = GiAddFloat32(value, lr.value);
        return *this;
    }
    Vector operator-(const Vector& lr) {
        Vector dst;
        dst.value = GiSubtractFloat32(value, lr.value);
        return dst;
    }
    Vector& operator-=(const Vector& lr) {
        value = GiSubtractFloat32(value, lr.value);
        return *this;
    }
    Vector operator*(float lr) {
        Vector dst;
        dst.value = GiMultiplyScalerFloat32(value, lr);
        return dst;
    }
    Vector operator*(const Vector& lr) {
        Vector dst;
        dst.value = GiMultiplyFloat32(value, lr.value);
        return dst;
    }
    Vector& operator*=(const Vector& lr) {
        value = GiMultiplyFloat32(value, lr.value);
        return *this;
    }
    Vector& operator=(const Vector& lr) {
        value = lr.value;
        return *this;
    }
    Vector& operator=(const Vector&& lr) {
        value = std::move(lr.value);
        return *this;
    }
    Vector operator-() {
        Vector dst;
        dst.value = -value;
        return dst;
    }
};

template <>
struct Vector<float, 8> {
    GI_FLOAT32_V2_t value;
    Vector() {}
    Vector(const float v) {
        value.val[0] = GiBroadcastFloat32(v);
        value.val[1] = GiBroadcastFloat32(v);
    }
    Vector(const Vector& lr) { value = lr.value; }
    Vector(const Vector&& lr) { value = std::move(lr.value); }
    Vector(const GI_FLOAT32_V2_t& v) { value = v; }
    static Vector load(const float* addr) {
        Vector v;
#if defined(GI_TEST_NAIVE)
        v.value.val[0] = GiLoadFloat32(addr);
        v.value.val[1] = GiLoadFloat32(addr + 4);
#elif defined(__arm__) || defined(__aarch64__)
        v.value = vld1q_f32_x2(addr);
#else
        v.value.val[0] = GiLoadFloat32(addr);
        v.value.val[1] = GiLoadFloat32(addr + 4);
#endif
        return v;
    }
    static void save(float* addr, const Vector& v) {
#if defined(GI_TEST_NAIVE)
        GiStoreFloat32(addr, v.value.val[0]);
        GiStoreFloat32(addr + 4, v.value.val[1]);
#elif defined(__arm__) || defined(__aarch64__)
        vst1q_f32_x2(addr, v.value);
#else
        GiStoreFloat32(addr, v.value.val[0]);
        GiStoreFloat32(addr + 4, v.value.val[1]);
#endif
    }

    void save(float* addr) { save(addr, *this); }
    Vector operator+(const Vector& lr) {
        Vector dst;
        dst.value.val[0] = GiAddFloat32(value.val[0], lr.value.val[0]);
        dst.value.val[1] = GiAddFloat32(value.val[1], lr.value.val[1]);
        return dst;
    }
    Vector& operator+=(const Vector& lr) {
        value.val[0] = GiAddFloat32(value.val[0], lr.value.val[0]);
        value.val[1] = GiAddFloat32(value.val[1], lr.value.val[1]);
        return *this;
    }
    Vector& add(const Vector& lr) {
        value.val[0] = GiAddFloat32(value.val[0], lr.value.val[0]);
        value.val[1] = GiAddFloat32(value.val[1], lr.value.val[1]);
        return *this;
    }
    Vector operator-(const Vector& lr) {
        Vector dst;
        dst.value.val[0] = GiSubtractFloat32(value.val[0], lr.value.val[0]);
        dst.value.val[1] = GiSubtractFloat32(value.val[1], lr.value.val[1]);
        return dst;
    }
    Vector& operator-=(const Vector& lr) {
        value.val[0] = GiSubtractFloat32(value.val[0], lr.value.val[0]);
        value.val[1] = GiSubtractFloat32(value.val[1], lr.value.val[1]);
        return *this;
    }
    Vector operator*(float lr) {
        Vector dst;
        dst.value.val[0] = GiMultiplyScalerFloat32(value.val[0], lr);
        dst.value.val[1] = GiMultiplyScalerFloat32(value.val[1], lr);
        return dst;
    }
    //! val + lr * n
    Vector& mla(const Vector& lr, float n) {
        value.val[0] = GiMultiplyAddScalarFloat32(value.val[0], lr.value.val[0], n);
        value.val[1] = GiMultiplyAddScalarFloat32(value.val[1], lr.value.val[1], n);
        return *this;
    }

    Vector operator*(const Vector& lr) {
        Vector dst;
        dst.value.val[0] = GiMultiplyFloat32(value.val[0], lr.value.val[0]);
        dst.value.val[1] = GiMultiplyFloat32(value.val[1], lr.value.val[1]);
        return dst;
    }
    Vector& operator*=(const Vector& lr) {
        value.val[0] = GiMultiplyFloat32(value.val[0], lr.value.val[0]);
        value.val[1] = GiMultiplyFloat32(value.val[1], lr.value.val[1]);
        return *this;
    }
    Vector& operator=(const Vector& lr) {
        value = lr.value;
        return *this;
    }
    Vector& operator=(const Vector&& lr) {
        value = std::move(lr.value);
        return *this;
    }
    Vector operator-() {
        Vector dst;
        dst.value.val[0] = -value.val[0];
        dst.value.val[1] = -value.val[1];
        return dst;
    }
};

}  // namespace fallback
}  // namespace megdnn

// vim: syntax=cpp.doxygen