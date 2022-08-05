#pragma once

#include "redstone.h"

// 判断字符是否为数字
bool isNum(char ch);

// 初始化红石地图
RsMap InitRsMap(int w, int h);

// 删除地图
void DeleteRsMap(RsMap* map);

// 重设地图大小
void ResizeRsMap(RsMap* map, int w, int h);

// 放置物品到地图
void PutObjectToRsMap(RsMap* map, int x, int y, int object_id, int direction = RS_TO_UP);

// 打开项目
RsMap OpenProject(const WCHAR* strFileName);

// 保存项目
bool SaveProject(RsMap map, const WCHAR* strFileName);

// 导入项目
// out		主项目
// in		被导入项目
// x, y		被导入项目位于主项目的左上角坐标
void ImportProject(RsMap* out, RsMap in, int x, int y);

// 在红石地图中绘制红石直线（不得弯曲），可指定用以填充红石直线的物品
// 返回是否绘制成功，绘制不成功的原因都是直线有弯曲
bool LineRsMap(RsMap* map, int x1, int y1, int x2, int y2, int object);

// 清除红石地图中的一块区域
void ClearRsMap(RsMap* map, int x1, int y1, int x2, int y2);

// 复制红石地图
void CopyRsMap(RsMap* pDst, RsMap* pSrc);

// 比较红石地图以重绘（比较依据：每个点的绘制效果）
// pChange		用于记录变更点的数组地址（记得在外部 delete[]）
// 注意：两地图长宽须相等
void CmpRsMapForRedraw(RsMap* pOld, RsMap* pNew, POINT** pChange, int* pCount);
