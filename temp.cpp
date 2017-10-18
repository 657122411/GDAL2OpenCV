////
//// Created by 陶剑浩 on 2017/10/13.
////
//
////通常的软件在处理降位时会存在一些问题，如曝光，出现空值等。根据常用的降位方法，如最简单的线性拉伸，分段拉伸以及对数变换和指数变换
//// ，结合常用影像的特点，使用百分比截断和指数（幂为0.7）变换将影像从16位降到8位，在抑制高光的同时保证了影像的对比度。
//int stretch_percent_16to8(const char *inFilename, const char *dstFilename)
//{
//    GDALAllRegister();
//    //为了支持中文路径
//    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
//    int src_height = 0;
//    int src_width = 0;
//    GDALDataset *poIn = (GDALDataset *)GDALOpen(inFilename,GA_ReadOnly);   //打开影像
//    //获取影像大小
//    src_width = poIn ->GetRasterXSize();
//    src_height = poIn ->GetRasterYSize();
//    //获取影像波段数
//    int InBands = poIn ->GetRasterCount();
//    //获取影像格式
//    GDALDataType eDataType = poIn -> GetRasterBand(1) -> GetRasterDataType();
//    //定义存储影像的空间参考数组
//    double adfInGeoTransform[6] = {0};
//    const char *pszWKT = NULL;
//    //获取影像空间参考
//    poIn ->GetGeoTransform(adfInGeoTransform);
//    pszWKT = poIn ->GetProjectionRef();
//    //创建文件
//    GDALDriver *poDriver = (GDALDriver *)GDALGetDriverByName("GTiff");
//    GDALDataset *poOutputDS = poDriver -> Create(dstFilename,src_width,src_height,InBands,GDT_Byte,NULL);
//
//    //设置拉伸后图像的空间参考以及地理坐标
//    poOutputDS -> SetGeoTransform(adfInGeoTransform);
//    poOutputDS -> SetProjection(pszWKT);
//    //读取影像
//
//    cout<<"16位影像降到8位影像处理..."<<endl;
//    for(int iBand = 0; iBand < InBands; iBand++ )
//    {
//        cout<<"正在处理第 "<<iBand + 1<<" 波段"<<endl;
//        //读取影像
//        uint16_t *srcData = (uint16_t *)malloc(sizeof(uint16_t) *src_width * src_height *1);
//        memset(srcData,0,sizeof(uint16_t ) * 1 *src_width * src_height);
//        int src_max = 0, src_min = 65500;
//        //读取多光谱影像到缓存
//        poIn ->GetRasterBand( iBand + 1) -> RasterIO( GF_Read, 0, 0, src_width, src_height , srcData + 0 * src_width * src_height,src_width,src_height, GDT_UInt16, 0, 0 );
//
//        //统计最大值
//        for (int src_row = 0; src_row < src_height; src_row ++)
//        {
//            for (int src_col = 0; src_col < src_width; src_col++)
//            {
//                uint16_t src_temVal = *(srcData + src_row * src_width + src_col);
//                if (src_temVal > src_max)
//                    src_max = src_temVal;
//                if(src_temVal < src_min )
//                    src_min = src_temVal;
//            }
//        }
//
//        double *numb_pix = (double *)malloc(sizeof(double)*(src_max+1));      //存像素值直方图，即每个像素值的个数
//        memset(numb_pix,0,sizeof(double) * (src_max+1));
//        //                 -------  统计像素值直方图  ------------         //
//
//        for (int src_row = 0; src_row < src_height; src_row ++)
//        {
//            for (int src_col = 0; src_col < src_width; src_col++)
//            {
//                uint16_t src_temVal = *(srcData + src_row * src_width + src_col);
//                *(numb_pix + src_temVal) += 1;
//            }
//        }
//
//        double *frequency_val = (double *)malloc(sizeof(double)*(src_max+1));           //像素值出现的频率
//        memset(frequency_val,0.0,sizeof(double)*(src_max+1));
//
//        for (int val_i = 0; val_i <= src_max; val_i++)
//        {
//            *(frequency_val + val_i) = *(numb_pix + val_i) / double(src_width * src_height);
//        }
//
//        double *accumlt_frequency_val = (double*)malloc(sizeof(double)*(src_max+1));   //像素出现的累计频率
//        memset(accumlt_frequency_val, 0.0,sizeof(double)*(src_max+1));
//
//        for (int val_i = 0; val_i <= src_max; val_i ++)
//        {
//            for (int val_j = 0; val_j < val_i; val_j ++ )
//            {
//                *(accumlt_frequency_val + val_i) += *(frequency_val + val_j);
//            }
//        }
//        //统计像素两端截断值
//        int minVal = 0, maxVal = 0;
//        for (int val_i = 1; val_i < src_max; val_i++)
//        {
//            double acc_fre_temVal0 = *(frequency_val + 0);
//            double acc_fre_temVal = *(accumlt_frequency_val + val_i);
//            if((acc_fre_temVal - acc_fre_temVal0) > 0.0015 )
//            {	minVal = val_i;
//                break;	}
//        }
//        for (int val_i = src_max-1; val_i > 0; val_i--)
//        {
//            double acc_fre_temVal0 = *(accumlt_frequency_val + src_max);
//            double acc_fre_temVal = *(accumlt_frequency_val + val_i);
//            if(acc_fre_temVal < (acc_fre_temVal0 - 0.00012) )
//            {	maxVal = val_i;
//                break;	}
//        }
//
//        for (int src_row = 0; src_row < src_height; src_row ++)
//        {
//            uint8_t *dstData = (uint8_t*)malloc(sizeof(uint8_t)*src_width);
//            memset(dstData, 0, sizeof(uint8_t)*src_width);
//            for (int src_col = 0; src_col < src_width; src_col++)
//            {
//                uint16_t src_temVal = *(srcData + src_row * src_width + src_col);
//                double stre_temVal = (src_temVal - minVal) / double(maxVal - minVal) ;
//                if(src_temVal < minVal)
//                {
//                    *(dstData + src_col) = (src_temVal) *(20.0/double(minVal)) ;
//                }
//                else if(src_temVal > maxVal)
//                {	stre_temVal = (src_temVal - src_min) / double(src_max - src_min);
//                    *(dstData + src_col) = 254;	}
//                else
//                    *(dstData + src_col) = pow(stre_temVal,0.7) * 250;
//
//            }
//            poOutputDS->GetRasterBand(iBand + 1)->RasterIO(GF_Write, 0,src_row,src_width,1,dstData,src_width,1,GDT_Byte,0,0);
//            free(dstData);
//        }
//
//        free(numb_pix);
//        free(frequency_val);
//        free(accumlt_frequency_val);
//        free(srcData);
//
//    }
//    GDALClose(poIn);
//    GDALClose(poOutputDS);
//
//    return 0;
//}