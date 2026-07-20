---
date: 2026-07-20
tags: [VTK, 数据模型, RectilinearGrid, 踩坑]
category: 疑问
---

# buildRectilinearGrid 无效果 —— 缺 SetDimensions

## 背景

实现了 `buildRectilinearGrid()`，设置了 X/Y/Z 坐标数组，也加了 CellData，但渲染窗口什么都不显示。

## 根因

**缺少 `SetDimensions()` 调用**。

VTK 的 RectilinearGrid 不会从坐标数组自动推导维度。光有 `SetXCoordinates` / `SetYCoordinates` / `SetZCoordinates` 不够，必须显式告诉它每个方向有多少个采样点。

## 修复

```cpp
// ❌ 之前
grid->SetXCoordinates(xCoords);
grid->SetYCoordinates(yCoords);
grid->SetZCoordinates(zCoords);

// ✅ 修复后
grid->SetXCoordinates(xCoords);
grid->SetYCoordinates(yCoords);
grid->SetZCoordinates(zCoords);
grid->SetDimensions(xCoords->GetNumberOfTuples(),
                    yCoords->GetNumberOfTuples(),
                    zCoords->GetNumberOfTuples());
```

## 对比 ImageData

ImageData 的 `SetDimensions()` 是**必须调用的**（否则没有网格），而 RectilinearGrid 的 SetDimensions 很容易被遗漏，因为你会误以为坐标数组已经隐含了维度信息。

## 我的理解

RectilinearGrid 的坐标数组只是"数据"，不定义结构。SetDimensions 才是告诉 VTK "这些坐标怎么组织成 IJK 网格"。

## 关联

- 相关章节：Chapter 1 — RectilinearGrid
- 相关笔记：[[2026-07-19-ImageData标量渲染可见性]] [[2026-07-20-Tuple命名设计]]
