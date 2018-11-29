# GDAL tiff to OpenCV Mat
使用GDAL库对遥感影像进行读取显示等操作，并将格式转为OpenCV的Mat，这样就可以使用OpenCV的算法:
使用gdal，直接GDAL读取分波段到内存直接OpenCV显示，之后出现纯黑需要进行归一化。需要先统计图像的值，然后选择一种方式进行拉伸，拉伸算法最后用的是envi的2%-98%的最小最大值线性拉伸。。。然后对拉伸处理后的数据导出成Mat


GDAL library is used to read and display remote sensing images, and the format is converted to OpenCV Mat. In this way, OpenCV algorithm can be used: using gdal, direct GDAL reads bands into memory for direct OpenCV display, and then pure black needs to be normalized. We need to count the image value first, then choose a way to stretch. The stretching algorithm uses the minimum maximum linear stretching of envi of 2%-98%. Then the data after stretching is deduced into Mat format.
