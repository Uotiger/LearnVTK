---
date: 2026-07-19
tags: [VTK, 数据模型, 渲染, ImageData]
category: 疑问
---

# VTK 标量数据的渲染可见性

## 背景

实现 `buildImageData()` 时发现：即使设置了 PointData 标量，渲染出来也看不到颜色变化。而 PolyData 就能直接看到。

## 核心问题

**Q：ImageData 的标量数据为什么看不到？**

A：vtkDataSetMapper 对 ImageData 的默认行为是渲染外表面（方盒子轮廓）。标量存在 PointData 里，但没有"表面"来着色。

```
vtkDataSetMapper 对不同数据类型的渲染：
├── PolyData → 渲染三角面片 → 标量映射到面片颜色 ✅
├── StructuredGrid → 渲染曲面网格 → 标量映射 ✅
└── ImageData → 只渲染外表面方盒子 → 标量看不到 ❌
```

## 怎么让 ImageData 标量"活"起来

| 方法 | 原理 | 适用场景 |
|------|------|----------|
| 体渲染 vtkSmartVolumeMapper | 直接渲染整个体积 | CT/MRI 可视化 |
| 等值面 vtkContourFilter | 提取等值面→PolyData | 等值面分析 |
| 切片 vtkImageSliceMapper | 显示某个截面 | 医学影像横截面 |

## 我的理解

标量数据本身是存在 PointData 里的，问题不在数据，**在于渲染方式**。vtkDataSetMapper 只关心"表面"，ImageData 的内部对它来说是黑盒。要看内部，需要换专门的 Mapper 或先提取表面。

这解释了为什么 VTK 有这么多 Mapper 类型——不同数据+不同可视化需求 = 不同的 Mapper。

## 待验证

- [ ] 用 vtkContourFilter 对 ImageData 提取等值面后，标量是否自动映射到颜色？
- [ ] 体渲染和表面渲染能否同时显示？

## 关联

- 相关章节：Chapter 1 — 数据模型，阶段二 Chapter 9 — 体渲染
- 相关笔记：[[2026-07-19-flywheel-startup]]
