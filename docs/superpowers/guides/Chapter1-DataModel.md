# Chapter 2 知识指南：VTK 数据模型

> 配套实现计划：[阶段一计划](../plans/2026-07-11-stage1-pipeline-foundation.md) — Task 1, 2

---

## 1. 为什么先学数据模型？

VTK 的一切都建立在数据之上。你写 `Mapper->SetInputData(data)` 之前，得先知道 `data` 到底是什么。很多 VTK 的诡异行为（渲染不出来、颜色不对、崩溃）根源都是数据结构没搞对。

> 一句话：**数据模型是 VTK 的地基。地基歪了，后面全歪。**

---

## 2. VTK 数据模型的类层次

```
vtkObject
  └── vtkDataObject          ← 所有数据的基类
        └── vtkDataSet        ← 有几何结构的数据
              ├── vtkImageData           (规则网格)
              ├── vtkRectilinearGrid     (矩形网格)
              ├── vtkStructuredGrid      (结构化网格)
              ├── vtkPolyData            (多边形数据)
              ├── vtkUnstructuredGrid    (非结构化网格)
              └── ... (vtkPointSet 等中间类)
```

**vtkDataObject vs vtkDataSet**：vtkDataObject 只带属性数据没有几何（比如一张表格），vtkDataSet 有几何（点坐标 + 单元连接）。你 99% 时间在跟 vtkDataSet 打交道。

> 参考：[VTK Book - Chapter 5: Data Representation](https://vtk.org/vtk-textbook/)

---

## 3. 五种 DataSet 详解

### 3.1 vtkImageData — 规则网格

**数据结构**：

```
三维数组，每个采样点等间距排列：
  origin(0,0,0) --- spacing(1,1,1) ---> 下一个点

  X方向 10 个点 → dimensions[0] = 10
  Y方向 8 个点  → dimensions[1] = 8
  Z方向 6 个点  → dimensions[2] = 6
```

**核心属性**（三个组合起来就能确定所有点的位置）：

| 属性 | 含义 | 默认值 |
|------|------|--------|
| `Origin` | 数据左下角的世界坐标 | (0, 0, 0) |
| `Spacing` | 相邻采样点的间距 | (1, 1, 1) |
| `Dimensions` | 每个方向的采样点数 | — |

**点的坐标怎么算？**

```
P(i,j,k) = Origin + (i * SpacingX, j * SpacingY, k * SpacingZ)
```

i, j, k 是整数索引，所以所有点落在规则的立方格点上。

**单元（Cell）**：N 个采样点 → N-1 个体素（voxel）。一个 `10×8×6` 的 ImageData 有 `9×7×5 = 315` 个体素单元。每个单元是一个立方体（或长方体，取决于 spacing）。

- 2D 时的单元叫 Pixel
- 3D 时的单元叫 Voxel

**优势**：内存只需要存 origin + spacing + dimensions（几十字节），而不是每个点的坐标。对 GB 级的 CT/MRI 数据这是决定性的。

**典型场景**：医学影像（DICOM → vtkImageData）、气象网格、体渲染。

> 参考：[VTK Doxygen - vtkImageData](https://vtk.org/doc/nightly/html/classvtkImageData.html)

---

### 3.2 vtkRectilinearGrid — 矩形网格

**和 ImageData 的区别**：采样间距不再均匀，但仍然是**轴对齐**的。

**数据结构**：

```
不存 spacing，改存三组数组：
  XCoordinates: [0.0, 1.0, 2.5, 4.0, 6.0, 8.5]  ← 6 个 X 坐标
  YCoordinates: [0.0, 1.5, 3.0, 5.0, 7.0]        ← 5 个 Y 坐标
  ZCoordinates: [0.0, 1.0, 2.0, 3.0, 4.0]        ← 5 个 Z 坐标
```

点的坐标：

```
P(i,j,k) = (XCoords[i], YCoords[j], ZCoords[k])
```

**为什么需要它？** 比如你要模拟边界层效应——靠近壁面的地方网格需要加密，远离壁面可以放松。这时 X 方向靠近壁面 0.01mm 一个点，远离壁面 1mm 一个点。ImageData 做不到，RectilinearGrid 能。

**性能**：比 ImageData 多占一点内存（要存三组坐标数组），但仍然远少于显式存每个点。

> 参考：[VTK Doxygen - vtkRectilinearGrid](https://vtk.org/doc/nightly/html/classvtkRectilinearGrid.html)

---

### 3.3 vtkStructuredGrid — 结构化网格

**和 RectilinearGrid 的区别**：点不再轴对齐，每个点都要显式给定坐标。**拓扑仍然由 IJK 索引推导。**

**数据结构**：

```
dimensions: [8, 6, 4]  ← 8×6×4 = 192 个点
points: 显式存 192 个 (x,y,z)
```

点的拓扑关系（哪个点在哪个点的"右边"）由 IJK 索引自动推导，不需要显式存连接表。这是它和非结构化网格的关键区别。

**你什么时候需要它？**

想象一个机翼表面的贴体网格——网格要"弯"成机翼的形状。点的坐标全变了，但 IJK 拓扑仍是有序的（第 (i,j,k) 个点的邻居始终是 (i±1, j±1, k±1)）。

这也是 CFD（计算流体力学）中最常见的网格类型之一。你工作中接触的 CGNS 数据很可能就是这种。

> 参考：[VTK Doxygen - vtkStructuredGrid](https://vtk.org/doc/nightly/html/classvtkStructuredGrid.html)

---

### 3.4 vtkPolyData — 多边形数据

**这是 VTK 中最重要、最常用、最灵活的数据类型。** 把它搞透了，后面 80% 的工作都顺畅。

**数据结构**：

```
Points:         [P0, P1, P2, ..., Pn]          ← 顶点坐标
Verts:          离散点（粒子、标注点）
Lines:          线段（流线、轮廓线）
Polys:          多边形（三角面片、四边形面片）
Triangle Strips: 三角带（高效的三角形组织方式）
```

**关键点**：PolyData 是**表面模型**，不是体模型。它存的是表面三角形，不存内部结构。你拿它渲染一个球，球内部是空的。

**单元连接表**：

```
Polys → vtkCellArray → [3, 0, 1, 2,  3, 0, 2, 3,  ...]
                          ↑            ↑
                    第一个三角形    第二个三角形
                    (顶点: 0,1,2)   (顶点: 0,2,3)
```

vtkCellArray 的存储格式：`[n0, id0, id1, ..., id(n0-1), n1, id0, id1, ...]` — 每个单元先写顶点个数，再跟顶点索引。

**vtkPolyData 的应用场景**：
- 等值面提取后的结果（Marching Cubes 输出就是 PolyData）
- 切面结果
- 从 CAD 导入的 STL/OBJ 文件
- 任何面渲染的数据

> 参考：[VTK Doxygen - vtkPolyData](https://vtk.org/doc/nightly/html/classvtkPolyData.html)，[VTK Examples - PolyData](https://examples.vtk.org/site/Cxx/PolyData/)

---

### 3.5 vtkUnstructuredGrid — 非结构化网格

**最通用、最灵活、也最耗内存的类型。**

**数据结构**：

```
Points: 显式存所有点坐标
Cells:  每个单元显式存储：单元类型 + 顶点列表
```

**支持的单元类型**（每种有一个 VTK 常量）：

| 类型 | VTK 常量 | 点数 | 形状 |
|------|---------|------|------|
| 顶点 | VTK_VERTEX | 1 | 点 |
| 线段 | VTK_LINE | 2 | 线 |
| 三角形 | VTK_TRIANGLE | 3 | 面 |
| 四边形 | VTK_QUAD | 4 | 面 |
| 四面体 | VTK_TETRA | 4 | 体 |
| 六面体 | VTK_HEXAHEDRON | 8 | 体 |
| 三棱柱 | VTK_WEDGE | 6 | 体 |
| 金字塔 | VTK_PYRAMID | 5 | 体 |
| ... | ... | ... | ... |

**为什么非结构化网格内存最大？**

```
ImageData 存一个体素:   0 字节连接信息（用 IJK 隐式推导）
UnstructuredGrid 存一个四面体: 4 字节单元类型 + 4 × 8 字节顶点 ID = 36 字节
                                                      ↑ (vtkIdType = 64位)
1000 万四面体 → 360 MB 仅连接信息（还不算点坐标）
```

**什么时候必须用它？**
- 混合单元类型（四面体 + 六面体 + 棱柱 混在一起）
- 从真实 CAD/CFD 软件导出的网格数据
- 你的 CGNS 文件加载后大部分就是 UnstructuredGrid

> 参考：[VTK Doxygen - vtkUnstructuredGrid](https://vtk.org/doc/nightly/html/classvtkUnstructuredGrid.html)，[VTK 单元类型列表](https://vtk.org/doc/nightly/html/vtkCellType_8h.html)

---

## 4. 属性数据：Point Data vs Cell Data

这是最容易搞混的概念。一句话说清楚：

```
Point Data = 存在顶点上 → 面内插值 → 渲染结果"平滑"
Cell Data  = 存在单元上 → 面内不变 → 渲染结果"锯齿状"
```

### 直观对比

想象一个三角形，三个顶点分别存温度 0°C、50°C、100°C：
- **Point Data**：面内任意点的温度 = 三个顶点的加权插值 → 连续渐变色
- **Cell Data**：整个三角形统一存一个温度值 50°C → 三角形内纯色，边界有跳变

**你什么时候用哪个？**

- 节点解（Node Solution）→ Point Data
- 单元解（Element Solution）→ Cell Data
- 渲染云图 → Point Data 更好看（平滑）
- 数据精度分析 → Cell Data 更准确（不引入插值误差）

### 访问属性的代码范式

```cpp
// 获取 Point Data
auto pd = dataset->GetPointData();
auto scalars = pd->GetScalars();  // 返回 vtkDataArray*
if (scalars) {
    double val = scalars->GetComponent(pointId, 0);  // 第 0 个分量
}

// 获取 Cell Data
auto cd = dataset->GetCellData();
auto scalars = cd->GetScalars();

// 获取某个数组（不一定是标量，可以是矢量/张量）
auto arr = pd->GetArray("Velocity");  // 按名字获取
```

**常见陷阱**：`GetScalars()` 返回默认激活的标量数组，可能为 nullptr！安全的做法是先检查：

```cpp
auto scalars = pd->GetScalars();
if (!scalars) {
    // 没有默认标量，手动选一个
    scalars = pd->GetArray("MyData");
}
```

> 参考：[VTK Doxygen - vtkPointData](https://vtk.org/doc/nightly/html/classvtkPointData.html)，[vtkCellData](https://vtk.org/doc/nightly/html/classvtkCellData.html)

---

## 5. 五种 DataSet 的选择决策树

```
你要存什么数据？
│
├── 规则采样，等间距？（CT/MRI）
│   → vtkImageData
│
├── 规则采样，但间距不均匀？（边界层网格）
│   → vtkRectilinearGrid
│
├── 轴对齐，但网格弯曲了？（贴体结构化网格）
│   → vtkStructuredGrid
│
├── 只有表面，没有内部？（三角面片、STL、等值面结果）
│   → vtkPolyData
│
└── 混合单元类型？不规则的内部体网格？（CFD 导出数据）
    → vtkUnstructuredGrid
```

---

## 6. 渲染管线怎么用这些数据

```
vtkDataSet (任何一种)
    │
    ▼
vtkDataSetMapper  ← 把 DataSet 转成 GPU 可渲染的图元
    │
    ▼
vtkActor          ← 附加颜色/材质/变换
    │
    ▼
vtkRenderer       ← 场景管理
    │
    ▼
vtkRenderWindow   ← 窗口
```

**重要**：`vtkDataSetMapper` 对 ImageData 画的是体素的表面，显示出来像一个方盒子；要用 `vtkSmartVolumeMapper` 才能做真正的体渲染（阶段二 Chapter 9 会学到）。

---

## 7. 调试技巧

**"为什么不显示？" 排查清单**：

1. `dataset->GetNumberOfPoints()` > 0？（数据确实有点？）
2. `mapper->SetInputData(dataset)` 调了吗？
3. `actor->SetMapper(mapper)` 调了吗？
4. `renderer->AddActor(actor)` 调了吗？
5. `renderer->ResetCamera()` 调了吗？（相机可能在很远的地方）
6. 点的坐标范围合理吗？（如果所有点都在 (1e10, 1e10, 1e10)，ResetCamera 后还是看不到）
7. Actor 的颜色和背景色一样吗？

**"颜色为什么不对？" 排查清单**：

1. ScalarVisibility 开了吗？`mapper->ScalarVisibilityOn()`
2. 有 Scalar 数据吗？`dataset->GetPointData()->GetScalars()` 不为 null？
3. LookupTable 的范围和数据的范围匹配吗？

---

## 8. 你的实践任务（Task 1 + 2）

看了上面的知识后，按计划文件实现：

1. **Task 1** — 搭骨架：CMakeLists.txt, main.cpp, DataModelDemo 窗口类（UI + 渲染器）
2. **Task 2** — 实现 5 个 build 方法，每个方法手动构造一种 DataSet，加属性数据

**验证标准**：
- 下拉框切换 5 种数据集，每种都能在窗口中看到
- 窗口顶部标签显示数据集类型名 + 点数 + 单元数
- 自行推导并验证：为什么 ImageData(10,8,6) 是 10×8×6=480 个点，但单元数是 9×7×5=315？

---

## 参考资源汇总

| 资源 | 链接 |
|------|------|
| VTK 官方文档 | https://vtk.org/documentation/ |
| VTK Doxygen (API) | https://vtk.org/doc/nightly/html/classes.html |
| VTK Examples (C++) | https://examples.vtk.org/site/Cxx/ |
| VTK Book (免费在线) | https://vtk.org/vtk-textbook/ |
| VTK 单元类型速查 | https://vtk.org/doc/nightly/html/vtkCellType_8h.html |
| vtkDataSet 类层次 | https://vtk.org/doc/nightly/html/classvtkDataSet.html |

---

开始写代码吧。搞不定或跑不通直接贴错误信息给我。
