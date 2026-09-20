TDSG Raster Engine
===============

Overview
--------

这是 TDSG 软光栅渲染引擎，作学习与研究之用。

这个渲染器没有引入第三方库。输入几何体，输出 `.tga` 图像文件（没有窗口）。

> TGA（Truevision Graphics Adapter）是美国 Truevision 公司为自己的显示卡开发的图像文件格式，文件扩展名为 `.tga`。由于格式简单、支持真彩色和 Alpha 通道，TGA 长期应用于计算机图形学、游戏开发和数字内容制作领域。



管线架构
----
虽然是软光栅，但是此渲染器相对完整地模拟了一个现代 GPU 渲染管线：

  * **应用阶段：** 场景初始化、设置渲染状态、组织 draw call
  * **几何阶段：** 顶点着色器、Sutherland-Hodgman 裁剪算法*、透视除法*、屏幕映射（viewport 变换）*
  * **光栅化阶段：** 覆盖测试*、透视校正的属性插值*、Early-Z*
  * **像素阶段：** 像素着色器（或称片元着色器）
  * 最后写入 framebuffer

> *这部分在现代渲染管线里（例如 Unity 等游戏引擎）基本上是不可见的，只能**配置**一极小部分（例如深度测试的开关状态）。

draw call 的状态组织部分在 `core/scene.cpp` 里的 `render()`，而填充 vertex 和 index buffer 在 `core/rasterizer.cpp` 的 `draw()`，这个有点乱，但是仍利于理解渲染管线的 draw call 是怎么一回事。

光栅化渲染在 `core/rasterizer.cpp`。

`math/` 是数学库，里面定义了线性代数运算。

`image/` 负责帧缓冲和 TGA 图像。

`io/` 负责加载外部的 `.obj` 模型文件。

### 杂项
#### 坐标系约定
**右手系，相机看向 $-z$。** NDC 的 $z$ 落在 $[-1, 1]$，`clip.w` 就是 view space 深度。矩阵是行主序，`M * v` 是矩阵乘列向量。

#### 统一属性插值
**光栅化器插值的对象是 `Fragment`** ，它对光栅化器而言完全不透明，我们重载了 `operator+` 和 `operator*` 运算符，可直接对 `Fragment` 整体进行插值运算，这使得我们在添加属性的时候不用去改光栅化器。

#### 着色器系统
`Shader::vertex()` 有默认实现（MVP 变换 + 传顶点属性），在需要顶点动画时可覆写，而 `Shader::fragment()` 是纯虚的。

#### 模型资源管理
`Object` 只借用 `Mesh` 的指针，不持有。往 `std::vector<Mesh>` 里 `push_back` 会让先前取到的地址失效，所以要先把 mesh 全部建完，再加 object。

构建
----
- Windows (amd64)
在 VS Code 里下载 CMake 插件，选用 MSVC 编译器，点左下角生成，然后执行
- macOS (aarch64)
同样在 VS Code 里下插件，但是使用 Apple clang 编译器，点击生成。

此时在 `build\` 文件夹下就有 `tdsg_raster_engine` 可执行程序了。



调试
----

渲染完成后会打印一行统计信息：

    wrote hall.tga  (29 objects, 9424 triangles, 1827 clipped, 968620 fragments)

`triangles` 是提交的三角形数，`clipped` 是被裁剪器动过的三角形（包括整个丢弃的），`fragments` 是通过深度测试、真正进入像素着色器的片元数量。

> `clipped` 变成 0 意味着模型根本没进视锥，或者 MVP 变换写错了。

`--no-clip` 关掉裁剪，用来确认裁剪器确实在工作，需要一个穿过屏幕的模型。

换着色器：

    --shader normal     输出法线信息
    --shader depth      输出深度
    --shader flat       顶点色
    --shader texture    纹理贴图
