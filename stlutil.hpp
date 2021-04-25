/**
 * @file stlutil.hpp
 * @brief STLファイルを地図などに使うためのユーティリティ
 * @author Shota Minami
 */
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

namespace stlutil {

    /**
     * 三次元上の点を表す構造体
     */
    struct STLVector {
        float x; // x座標
        float y; // y座標
        float z; // z座標
    };

    /**
     * 三次元上の線分を表す構造体
     */
    struct STLSegment {
        STLVector p; // 始点
        STLVector q; // 終点
    };

    /**
     * 三次元上のポリゴンを表す構造体
     * @details 法線ベクトルの向きと頂点の右ねじの向きは一致します
     */
    struct STLPolygon {
        STLVector normal; // 法線ベクトル
        STLVector a; // 頂点1
        STLVector b; // 頂点2
        STLVector c; // 頂点3
    };

    /**
     * STLファイルのデータを保持する構造体
     */
    struct STLReader {
    private:
        std::string header_; // STLファイルの先頭80byte
        std::vector<STLPolygon> polygons_; // ポリゴンの配列

        bool valid = false; // 読み取りが正常に行えたかどうか

    public:
        STLReader(const STLReader&) = delete;

        /**
         * STLファイルを読み込んでポリゴンの読み出しを行ないます
         * @param path 読み込むSTLファイルへのパス
         */
        explicit STLReader(const std::string &path) {
            std::ifstream stlfile(path, std::ios::in | std::ios::binary);
            if (!stlfile) {
                std::cerr << "STLReader::STLReader() Error: Cannot open file `" << path << "`." << std::endl;
                return;
            }
            stlfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            header_.resize(80);
            stlfile.read(header_.data(), 80); // 先頭80byteの読み取り
            uint32_t size;
            stlfile.read(static_cast<char *>(static_cast<void *>(&size)), 4); // ポリゴンの数の読み取り
            polygons_.resize(size);
            for (auto& [ normal, a, b, c ] : polygons_) {
                {
                    auto& [ x, y, z ] = normal;
                    // 法線ベクトルの読み取り
                    stlfile.read(static_cast<char*>(static_cast<void *>(&x)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&y)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&z)), 4);
                }
                {
                    auto& [ x, y, z ] = a;
                    // 三角形の頂点1の読み取り
                    stlfile.read(static_cast<char*>(static_cast<void *>(&x)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&y)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&z)), 4);
                }
                {
                    auto& [ x, y, z ] = b;
                    // 三角形の頂点2の読み取り
                    stlfile.read(static_cast<char*>(static_cast<void *>(&x)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&y)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&z)), 4);
                }
                {
                    auto& [ x, y, z ] = c;
                    // 三角形の頂点3の読み取り
                    stlfile.read(static_cast<char*>(static_cast<void *>(&x)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&y)), 4);
                    stlfile.read(static_cast<char*>(static_cast<void *>(&z)), 4);
                }
                stlfile.ignore(2); // 2byteスキップ
            }

            valid = true;
        }

        /**
         * @return 読み取りが正常に行えていたらtrue，そうでなければfalse
         */
        explicit operator bool() const noexcept {
            return valid;
        }

        /**
         * @return ポリゴンが格納された配列の先頭へのイテレータ
         */
        [[nodiscard]]
        const auto begin() const noexcept {
            return polygons_.begin();
        }

        /**
         * @return ポリゴンが格納された配列の終端へのイテレータ
         */
        [[nodiscard]]
        const auto end() const noexcept {
            return polygons_.end();
        }

        /**
         * @return STLファイルの先頭の文字列
         * @details 読み取り専用
         */
        [[nodiscard]]
        const auto &header() const noexcept {
            return header_;
        }

        /**
         * @return ポリゴンの配列への参照
         * @details 読み取り専用
         */
        [[nodiscard]]
        const auto &polygons() const noexcept {
            return polygons_;
        }
    };

    namespace internal {

        /**
         * 線分と ax + by + cz + d = 0 で表される平面の交点の媒介変数の値を求めます
         * @param segment 線分
         * @param a 平面の式のxの係数
         * @param b 平面の式のyの係数
         * @param c 平面の式のzの係数
         * @param d 平面の式の定数
         * @return 交点の媒介変数の値
         */
        [[nodiscard]]
        static inline float segment_plane_intersection(
            const STLSegment &segment,
            const float a, const float b, const float c, const float d
        ) noexcept {
            const auto& [ x1, y1, z1 ] = segment.p;
            const auto& [ x2, y2, z2 ] = segment.q;
            return -(a * x1 + b * y1 + c * z1 + d)
                / (a * (x2 - x1) + b * (y2 - y1) + c * (z2 - z1));
        }

        /**
         * 線分の内分点を求めます
         * @param segment 線分
         * @param t 内分比 1 - t : t
         * @return 内分点
         */
        [[nodiscard]]
        static inline STLVector point_on_line(
            const STLSegment &segment,
            const float t
        ) noexcept {
            const auto& [ x1, y1, z1 ] = segment.p;
            const auto& [ x2, y2, z2 ] = segment.q;
            return {
                (1 - t) * x1 + t * x2,
                (1 - t) * y1 + t * y2,
                (1 - t) * z1 + t * z2
            };
        }
    }

    /**
     * ポリゴンを ax + by + cz + d = 0 で表される平面でスライスします
     * @param polygons スライスの対象となるポリゴンの配列
     * @param a 平面の式のxの係数
     * @param b 平面の式のyの係数
     * @param c 平面の式のzの係数
     * @param d 平面の式の定数
     * @return スライスして得られた線分の配列
     */
    [[nodiscard]]
    std::vector<STLSegment> slice_polygons_at(
        const std::vector<STLPolygon> &polygons,
        const float a, const float b, const float c, const float d
    ) noexcept {
        std::vector<STLSegment> res;
        for (const auto& [ ignore, p, q, r ] : polygons) {
            float s = internal::segment_plane_intersection({ p, q }, a, b, c, d);
            float t = internal::segment_plane_intersection({ q, r }, a, b, c, d);
            float u = internal::segment_plane_intersection({ r, p }, a, b, c, d);

            if (0 <= s && s < 1 && 0 <= t && t < 1) {
                STLVector alpha = internal::point_on_line({ p, q }, s);
                STLVector beta  = internal::point_on_line({ q, r }, t);
                res.push_back({ alpha, beta });
            } else if (0 <= t && t < 1 && 0 <= u && u < 1) {
                STLVector alpha = internal::point_on_line({ q, r }, t);
                STLVector beta  = internal::point_on_line({ r, p }, u);
                res.push_back({ alpha, beta });
            } else if (0 <= u && u < 1 && 0 <= s && s < 1) {
                STLVector alpha = internal::point_on_line({ r, p }, u);
                STLVector beta  = internal::point_on_line({ p, q }, s);
                res.push_back({ alpha, beta });
            }
        }
        return res;
    }

    /**
     * ポリゴンをx軸に垂直な平面でスライスします
     * @param polygons スライスの対象となるポリゴンの配列
     * @param x スライスを行うx座標
     * @return スライスして得られた線分の配列
     */
    [[nodiscard]]
    std::vector<STLSegment> slice_polygons_at_x(
        const std::vector<STLPolygon> &polygons,
        const float x
    ) noexcept {
        return slice_polygons_at(polygons, 1, 0, 0, -x);
    }

    /**
     * ポリゴンをy軸に垂直な平面でスライスします
     * @param polygons スライスの対象となるポリゴンの配列
     * @param y スライスを行うy座標
     * @return スライスして得られた線分の配列
     */
    [[nodiscard]]
    std::vector<STLSegment> slice_polygons_at_y(
        const std::vector<STLPolygon> &polygons,
        const float y
    ) noexcept {
        return slice_polygons_at(polygons, 0, 1, 0, -y);
    }

    /**
     * ポリゴンをz軸に垂直な平面でスライスします
     * @param polygons スライスの対象となるポリゴンの配列
     * @param z スライスを行うz座標
     * @return スライスして得られた線分の配列
     */
    [[nodiscard]]
    std::vector<STLSegment> slice_polygons_at_z(
        const std::vector<STLPolygon> &polygons,
        const float z
    ) noexcept {
        return slice_polygons_at(polygons, 0, 0, 1, -z);
    }
}
