# STLUtil

STLをC++で読み込むヘッダオンリーライブラリです．STLファイルの読み込みと，地図として利用するための断面図の作成ができます．

## インストール

`stlutil.hpp` をincludeディレクトリに設置する．それだけ！

## 使い方

### ポリゴンのデータを用いた処理

```cpp
int main() {
    stlutil::STLReader reader("path/to/your/stl");
    if (!reader) {
        std::cout << "読み込みに失敗" << std::endl;
        return;
    }

    for (const auto& [ normal, a, b, c ] : reader) {
        // やりたい処理を書く．Rvizに送信したりとか
        // normal: 法線ベクトル
        // a, b, c: 三角形の頂点
    }
}
```

### 断面図の作成

```cpp
int main() {
    stlutil::STLReader reader("path/to/your/stl");
    if (!reader) {
        std::cout << "読み込みに失敗" << std::endl;
        return;
    }

    // 平面 z = 50 で断面図を作成する．
    const auto res = stlutil::slice_polygons_at_z(reader.polygons(), 50);
    for (const auto& [ p, q ] : res) {
        // やりたい処理を書く．Eigenのベクトルに変換したりとか
    }
}
```

###

## 参考

- [stlファイル(バイナリ)を読み込む](https://fortefibre.esa.io/posts/1304)
- [LRF向けに当初作られた実装](https://gitlab.fortefibre.net/ynakanishi/lrf_localization/blob/feat/stl/include/stl_loader.h)
