---
date: 2026-07-20
tags: [VTK, 渲染, PointData, CellData, LookupTable]
category: 对话沉淀
---

# PointData vs CellData 的渲染差异 + LookupTable 范围

## 背景

实现 `buildImageData()` 时发现：明明给 480 个点设了 Distance 标量，渲染出来的方块却全是蓝色，看不到色差。而 RectilinearGrid 的 CellData 色块分明。

## PointData → 平滑插值 → 边界糊掉

```cpp
image->GetPointData()->SetScalars(scalars);  // PointData
```

| PointData 特征 | 效果 |
|----------------|------|
| 存顶点上 | 顶点间 GPU 线性插值 |
| 渲染结果 | 连续渐变色 |
| 视觉 | 色差"糊"掉，看不到明显边界 |

**更重要的是**：`vtkDataSetMapper` 只渲染 ImageData 的外表面。Distance 标量定义在整个体积上（0~6.3），但外表面点的值全在 4~6 的窄区间 → LookupTable 映射到同一色段 → 全蓝。

## CellData → 平涂 → 边界分明

```cpp
grid->GetCellData()->SetScalars(scalars);  // CellData
```

| CellData 特征 | 效果 |
|---------------|------|
| 存单元上 | 单元内纯色，不插值 |
| 渲染结果 | 色块状 |
| 视觉 | 边界分明，差异一目了然 |

RectilinearGrid 的非均匀间距让相邻单元体积差好几倍 → 色差明显。

## LookupTable 范围陷阱

即使 PointData 值有差异，如果 LookupTable 的 range 没对齐，颜色也会挤压到一起：

```cpp
// 修复：显式从数据读取范围，强制对齐
double range[2];
m_currentDataset->GetScalarRange(range);
mapper->SetScalarRange(range);
```

## 验证实验

把 Distance 标量换成简单的 X 坐标索引（0→9），再配上 `SetScalarRange`，方块表面立刻出现从左到右的清晰渐变色带——证明 PointData 完全生效，只是之前标量设计 + 范围问题藏住了效果。

## 我的理解

1. **PointData = 连续色，CellData = 色块**——选择取决于你要"好看"还是"准确"
2. **LookupTable 范围是隐藏坑**——Mapper 自动检测有时不准，显式设置最保险
3. **表面渲染 vs 体积数据**——ImageData 是体数据，Mapper 只看外表面，内部标量被"藏"起来了

## 关联

- 相关章节：Chapter 1 — 数据模型
- 相关笔记：[[2026-07-19-ImageData标量渲染可见性]] [[2026-07-20-RectGrid缺SetDimensions无效]]
