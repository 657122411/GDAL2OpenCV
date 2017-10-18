/*
    遥感影像通常是TIFF数据，OPENCV读取TIFF会出问题，所以采用了先用GDAL读取数据以及影像信息，
再转换为OPENCV的Mat类型，这样结合了GDAL强大的支持多种数据格式的能力，又能方便地调用OPENCV的算法函数。
思路就是：根据文件名获得其GDALDataset数据集，然后分波段（波段相当于通道）存储在格式为Vector<cv::Mat>的容器内，
最后利用MAT的Merge函数，对通道数据进行组合。以上方法适合任意波段数据，对多通道影像，如遥感影像中多光谱和高光谱数据比较实用。
*/

#include<opencv2/highgui/highgui.hpp>;
#include<gdal_priv.h>;
#include<iostream>;
using namespace cv;
using namespace std;

//所谓2%-98%最大最小值拉伸就是先统计图像的灰度直方图，然后计算灰度的累积分布函数，CDF=0.02的时候取值为min,
// CDF=0.98的时候取值为max,再进行最大最小值拉伸小于min的全部为0，大于max的为255，这样做的好处就是可以把像素值过大或者过小的噪声剔除，
// 从而可以增强图像的显示效果
typedef unsigned short ushort;
typedef unsigned char uchar;


/**
直接最大最小值拉伸，小于最小值的设为0，大于最大值的设为255
@param pBuf 保存16位影像数据的数组，该数组一般直接由Gdal的RasterIO函数得到
@param dstBuf 保存8位影像数据的数组，该数组保存拉伸后的8
@param width 图像的列数
@param height 图像的行数
@param minVal 最小值
@param maxVal 最大值
*/
void MinMaxStretch(ushort *pBuf,uchar * dstBuf,int bufWidth,int bufHeight,double minVal,double maxVal)
{
    ushort data;
    uchar result;
    for (int x=0;x<bufWidth;x++)
    {
        for (int y=0;y<bufHeight;y++)
        {
            data=pBuf[x*bufHeight+y];
            result=(data-minVal)/(maxVal-minVal)*255;
            dstBuf[x*bufHeight+y]=result;
        }
    }
}

/**
2%-98%最大最小值拉伸，小于最小值的设为0，大于最大值的设为255
@param pBuf 保存16位影像数据的数组，该数组一般直接由Gdal的RasterIO函数得到
@param dstBuf 保存8位影像数据的数组，该数组保存拉伸后的8
@param width 图像的列数
@param height 图像的行数
@param minVal 最小值
@param maxVal 最大值
*/
void MinMaxStretchNew(ushort *pBuf,uchar *dstBuf,int bufWidth,int bufHeight,double minVal,double maxVal)
{
    ushort data;
    uchar result;
    for (int x=0;x<bufWidth;x++)
    {
        for (int y=0;y<bufHeight;y++)
        {
            data=pBuf[x*bufHeight+y];
            if (data>maxVal)
                result=255;
            else if (data<minVal)
                result=0;
            else
                result=(data-minVal)/(maxVal-minVal)*255;
            dstBuf[x*bufHeight+y]=result;
        }
    }
}

/**
计算灰度累积直方图概率分布函数，当累积灰度概率为0.02时取最小值，0.98取最大值
@param pBuf 保存16位影像数据的数组，该数组一般直接由Gdal的RasterIO函数得到
@param width 图像的列数
@param height 图像的行数
@param minVal 用于保存计算得到的最小值
@param maxVal 用于保存计算得到的最大值
*/
void HistogramAccumlateMinMax16S(ushort *pBuf,int width,int height,double *minVal,double *maxVal)
{
    double p[1024],p1[1024],num[1024];

    memset(p,0,sizeof(p));
    memset(p1,0,sizeof(p1));
    memset(num,0,sizeof(num));

    long wMulh = height * width;

    //计算灰度分布
    for(int x=0;x<width;x++)
    {
        for(int y=0;y<height;y++)
        {
            ushort v=pBuf[x*height+y];
            num[v]++;
        }
    }

    //计算灰度的概率分布
    for(int i=0;i<1024;i++)
        p[i]=num[i]/wMulh;

    int min=0,max=0;
    double minProb=0.0,maxProb=0.0;
    //计算灰度累积概率
    //当概率为0.02时，该灰度为最小值
    //当概率为0.98时，该灰度为最大值
    while(min<1024&&minProb<0.02)
    {
        minProb+=p[min];
        min++;
    }
    do
    {
        maxProb+=p[max];
        max++;
    } while (max<1024&&maxProb<0.98);

    *minVal=min;
    *maxVal=max;
}

//16bit直接转化为8Bit输出，也可以转化为OpenCV的Mat格式直接在OpenCV显示，或者在Qt中进行显示
void Create8BitImage(const char *srcfile,const char *dstfile)
{
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
    GDALDataset *pDataset=(GDALDataset *) GDALOpen( srcfile, GA_ReadOnly );
    int bandNum=pDataset->GetRasterCount();

    //获取影像大小
    int src_height = 0;
    int src_width = 0;
    src_width = pDataset ->GetRasterXSize();
    src_height = pDataset ->GetRasterYSize();


    GDALDriver *pDriver=GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset *dstDataset=pDriver->Create(dstfile,src_width,src_height,4,GDT_Byte,NULL);
    GDALRasterBand *pBand;
    GDALRasterBand *dstBand;

    //定义存储影像的空间参考数组
    double adfInGeoTransform[6] = {0};
    const char *pszWKT = NULL;
    //获取影像空间参考
    pDataset ->GetGeoTransform(adfInGeoTransform);
    pszWKT = pDataset ->GetProjectionRef();
    dstDataset -> SetGeoTransform(adfInGeoTransform);
    dstDataset -> SetProjection(pszWKT);


    //写入光栅数据
    ushort *sbuf= new ushort[src_width*src_height];
    uchar *cbuf=new uchar[src_width*src_height];
    for (int i=1,j=1;i<bandNum;i++,j++)
    {
        pBand=pDataset->GetRasterBand(i);
        pBand->RasterIO(GF_Read,0,0,src_width,src_height,sbuf,src_width,src_height,GDT_UInt16,0,0);
        int bGotMin, bGotMax;
        double adfMinMax[2];
        adfMinMax[0] = pBand->GetMinimum( &bGotMin );
        adfMinMax[1] = pBand->GetMaximum( &bGotMax );
        if( ! (bGotMin && bGotMax) )
            GDALComputeRasterMinMax((GDALRasterBandH)pBand, TRUE, adfMinMax);
        //MinMaxStretch(sbuf,cbuf,src_width,src_height,adfMinMax[0],adfMinMax[1]);

        double min,max;
        HistogramAccumlateMinMax16S(sbuf,src_width,src_height,&min,&max);
        MinMaxStretchNew(sbuf,cbuf,src_width,src_height,min,max);

        dstBand=dstDataset->GetRasterBand(j);
        dstBand->RasterIO(GF_Write,0,0,src_width,src_height,cbuf,src_width,src_height,GDT_Byte,0,0);
    }
    delete []cbuf;
    delete []sbuf;
    GDALClose(pDataset);
    GDALClose(dstDataset);
}


int main()
{
    //记录时间
    double time0 = static_cast<double>(getTickCount());

//    stretch_percent_16to8("../IMG1.TIF","temp1.tif");
    Create8BitImage("../IMG1.TIF","/Users/taojianhao/Documents/temp.tif");

    Mat srcimg;

    GDALDataset *poDataset;
    const char *pszFilename= "/Users/taojianhao/Documents/temp.tif";

    //使用前需要先进行注册驱动
    GDALAllRegister();
    poDataset = (GDALDataset*)GDALOpen(pszFilename,GA_ReadOnly);
    //获取影像的宽高，波段数
    int width = poDataset->GetRasterXSize();
    int height = poDataset->GetRasterYSize();
    int nband = poDataset->GetRasterCount();

    GDALRasterBand *bandData;
    float *p = new float[width*height];
    vector<Mat> imagesT;

    //遍历波段，读取到mat向量容器里.注意顺序
    for(int i = 1;i < nband;i++)
    {
        bandData = poDataset->GetRasterBand(i);
        GDALDataType DataType = bandData->GetRasterDataType();
        bandData->RasterIO(GF_Read,0,0,width,height,p,width,height,DataType ,0,0);


        Mat HT(height,width,CV_8U,p);

//        Mat show(HT.size(),CV_8UC1);
//        normalize(HT,show,0,255,CV_MINMAX);
//        show.convertTo(show,CV_8U);
//        namedWindow("nband",CV_WINDOW_AUTOSIZE);
//        imshow("nband",show);

        imagesT.push_back(HT.clone());

    }

    //多通道融合
    Mat Timg;
    Timg.create(width,height,CV_8UC3);
    merge(imagesT,Timg);

//    //进行归一化，映射到0-255
//    Mat result(Timg.size(),CV_8UC3);
//    normalize(Timg,result,0,255,CV_MINMAX);
//    result.convertTo(result,CV_8U);
    namedWindow("result",CV_WINDOW_AUTOSIZE);
    imshow("result",Timg);



    imwrite("output.jpg",Timg);

    GDALClose((GDALDatasetH)poDataset);
    imagesT.clear();

    //记录时间并输出
    time0 = ((double)getTickCount() - time0)/getTickFrequency();
    cout << "花费时间:"<<time0<<"秒"<<endl;

    cv::waitKey(0);
    std::system("pause");
    return 0;
}


