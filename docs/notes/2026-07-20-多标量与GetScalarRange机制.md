---
date: 2026-07-20
tags: [VTK, 标量, API设计, 渲染]
category: 疑问
---

# 多标量时 GetScalarRange 怎么生效

## 背景

代码中用 `GetScalarRange` + `SetScalarRange` 强制对齐色表范围。但如果 dataset 有多组标量（压力、速度、位移），`GetScalarRange` 读的是哪个的范围？

## 核心机制：默认标量

`GetScalarRange()` 只读**默认标量**（即最后一个被 `SetScalars()` 设置的数组）：

```cpp
auto pressure   = ...; pd->AddArray(pressure);
auto velocity   = ...; pd->AddArray(velocity);
auto displace   = ...; pd->AddArray(displacement);

pd->SetScalars(pressure);   // ← 设为默认标量
// GetScalarRange() → 只读 pressure 的范围
```

`AddArray` 只是"把数组放进去"，不影响默认标量。`SetScalars` 才是"激活"。

## 切换不同标量来着色

```cpp
// 方式一：改默认标量（全局生效，后续 GetScalarRange 跟着变）
dataset->GetPointData()->SetScalars(velocity);

// 方式二：Mapper 选择着色数组（只影响渲染）
mapper->SelectColorArray("Velocity");
mapper->SetScalarRange(velocity->GetRange());  // 手动设置对应范围
```

## 三个层次

```
DataSet
  └── PointData
        ├── "Pressure"     ← SetScalars 设它为默认 → GetScalarRange 读它
        ├── "Velocity"     ← 存在，不是默认
        └── "Displacement" ← 存在，不是默认

GetScalarRange()  → PointData::GetScalars() → 默认数组 → [min, max]
SelectColorArray("Velocity") → Mapper 忽略默认，直接用 Velocity
```

## 一句话总结

> SetScalars = 设默认，GetScalarRange 只读默认。想切换标量，用 SelectColorArray + 手动 SetScalarRange。

## 关联

- 相关笔记：[[2026-07-20-PointData与CellData渲染差异]]
