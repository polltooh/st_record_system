// GloopMeasureSpiDemo.cpp : Defines the entry point for the console application.
//
#include "gloop_measure_spi_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <fstream>
#include <sys/stat.h>

//==========================================================================
/*
class Graph {
public:
  float max_val;
  float min_val;
  int idx;
  cv::Mat_<int> buff;
  cv::Mat_<uchar> image;
  
  void init(const int buf_size, const cv::Size img_size) {
    max_val = -1e+10; min_val = 1e+10; idx = 0;
    buff.create(buf_size, 1);
    image.create(img_size);
  }
  void update(const float v) {
    buff(idx++) = v;
    if (idx >= buff.rows * buff.cols) {
      idx = 0;
    }
    max_val = cv::max(max_val, v);
    min_val = cv::min(min_val, v);
  }  
  const cv::Mat_<uchar> draw() {
    image = 0.0;
    int buff_size = buff.rows * buff.cols;
    for (int i = 0, j = idx; i < buff_size - 1; i++, j++) {
      int j1 = j, j2 = j + 1;
      if (j1 >= buff_size) { j1 -= buff_size; }
      if (j2 >= buff_size) { j2 -= buff_size; }
      float x1 = (static_cast<float>(image.cols) / buff_size) * i;
      float x2 = (static_cast<float>(image.cols) / buff_size) * i;
      float y1 = image.rows * (buff(j1) - min_val) / (max_val - min_val);
      float y2 = image.rows * (buff(j2) - min_val) / (max_val - min_val);
      cv::line(image, cv::Point2f(x1, y1), cv::Point2f(x2, y2),
	       cv::Scalar::all(255), 1, CV_AA);
      cv::circle(image, cv::Point2f(x2, y2), 1,
		 cv::Scalar::all(255), 1, CV_AA);
    }
    return image;
  }
};
*/
class Graph {
public:
  int64 num;
  float sum;
  float range;
  int idx;
  cv::Mat_<int> buff;
  cv::Mat_<uchar> image;
  
  void init(const int buf_size, const cv::Size img_size) {
    idx = 0; sum = 0; num = 0; range = 500;
    buff.create(buf_size, 1);
    image.create(img_size);
  }
  void update(const float v) {
    buff(idx++) = v;
    if (idx >= buff.rows * buff.cols) {
      idx = 0;
    }

    /*
    if (num > 100) {
      float alpha = 0.99;
      sum = (alpha * (sum / num) + (1.0f - alpha) * v) * num;
    } else {
      std::cout << num << std::endl;
      sum += v;
      num += 1;
    }
    */
    sum += v;
    num += 1;
  }  
  const cv::Mat_<uchar> draw() {
    image = 0.0;
    int buff_size = buff.rows * buff.cols;
    double avg = sum / num;
    /*
    for (int i = 0, j = idx; i < buff_size - 1; i++, j++) {
      int j1 = j, j2 = j + 1;
      if (j1 >= buff_size) { j1 -= buff_size; }
      if (j2 >= buff_size) { j2 -= buff_size; }
      float x1 = (static_cast<float>(image.cols) / buff_size) * i;
      float x2 = (static_cast<float>(image.cols) / buff_size) * i;
      float y1 = image.rows * ((buff(j1) - avg) / 4000 + 0.5);
      float y2 = image.rows * ((buff(j2) - avg) / 4000 + 0.5);
      cv::line(image, cv::Point2f(x1, y1), cv::Point2f(x2, y2),
	       cv::Scalar::all(255), 1, CV_AA);
      cv::circle(image, cv::Point2f(x2, y2), 1,
		 cv::Scalar::all(255), 1, CV_AA);
    }
    */
    image.row(image.rows / 2) = 255;
    for (int i = 0, j = idx; i < buff_size; i++, j++) {
      if (j >= buff_size) { j -= buff_size; }
      int x = (static_cast<float>(image.cols) / buff_size) * i;
      float vy = image.rows * ((buff(j) - avg) / range + 0.5);
      int y = cv::min(image.rows - 1, cv::max(0, int(vy)));
      if (y > image.rows / 2) {
	image(cv::Rect(x, image.rows / 2, 1, y - image.rows / 2)) = 255;
      } else {
	image(cv::Rect(x, y, 1, image.rows / 2 - y)) = 255;
      }
    }
    cv::rectangle(image, cv::Point(0, 0),
		  cv::Point(image.cols - 1, image.rows - 1),
		  CV_RGB(100,100,100), 2, CV_AA);
    return image;
  }
};
//==========================================================================
class StretchSensor {
private:
  spi_context_t* context;
  gloop_data_spidat2_t data;
public:
  int init(uint8_t deviceNum, uint8_t interfaceNum) {
    int32_t size = gloop_create_default_context_with_id(NULL, deviceNum,
							interfaceNum);
    clock_t start_time, end_time;
    double time_span;
    if (size < 0) {
      printf("error sizing context.\n");
      return -1;
    }
    context = (spi_context_t*)malloc(size);
	//size = gloop_create_default_context(context);
    size = gloop_create_default_context_with_id(context, deviceNum,
							interfaceNum);
    if (size < 0) {
      printf("error creating spi context.\n");
      return -1;
    }
    gloop_send_config_cmd2(context, GLOOP_ODR_500HZ, GLOOP_TRGR_CONT,
			   16, GLOOP_RES_001PF);
    return 0;
  }
  void grab(cv::Mat_<float> &val) {
    gloop_read_data2(context, &data);
    for (int i = 2; i < sizeof(data); i += 2) {
      double v =
	((((uint16_t) data.buf[i]) << 8) | ((uint16_t) data.buf[i + 1]));
      int k = (i - 2) / 2;
      val(k) = v;
    }
  }
};
//==========================================================================
int captureExpressions() {

  std::string odir = "jason";

  struct stat st = {0};
  if (stat(odir.c_str(), &st) == -1) {
    mkdir(odir.c_str(), 0700);
  }
  
  std::vector<std::string> expressions = {
    "eyebrow raised",
    "left eyebrow raised",
    "right eyebrow raised",
    "brow furrow",
    "sad eyebrows",
    "left wink ",
    "right wink",
    "nose wrinkle",
    "upper lip raiser",
    " smile",
    " left cheek raiser",
    " right cheek raiser"
  };
  cv::RNG rn(cv::getTickCount());

  int nsensors = 10;
  cv::Size imgSize(128, 128);
  std::vector<Graph> G(nsensors);
  for (int i = 0; i < nsensors; i++) {
    G[i].init(128, imgSize);
  }
  cv::Mat_<float> val(nsensors, 1);

  
  uint8_t deviceNum = 12;
  uint8_t interfaceNum = 0;
  StretchSensor sensor;
  if (sensor.init(deviceNum, interfaceNum) < 0) {
    std::cout << "Failed initialising sensor!" << std::endl;
    return -1;
  }
  
  for (int sample = 0;; sample++) {
    //sample expression order
    std::vector<int> eidx(expressions.size());
    for (size_t i = 0; i < expressions.size(); i++) { eidx[i] = i; }
    for (size_t i = 0; i < expressions.size(); i++) {
      size_t j = rn.uniform(0, expressions.size());
      int k = eidx[j]; eidx[j] = eidx[i]; eidx[i] = k;
    }
    cv::Mat_<float> measure(nsensors, expressions.size());
    bool quit = false;
    for (int ei = 0; ei < expressions.size(); ei++) {
      //std::cout << expressions[eidx[ei]] << std::endl;
      quit = false;
      while (1) {
	sensor.grab(val);
	for (int i = 0; i < nsensors; i++) { G[i].update(val(i)); }
	cv::Mat_<uchar> Image(2 * imgSize.height,
			      (nsensors / 2) * imgSize.width);
	for (int k = 0; k < nsensors; k++) {
	  int y = k / (nsensors / 2);
	  int x = k - y * (nsensors / 2);
	  const cv::Mat_<uchar> I = G[k].draw();
	  cv::Mat im = Image(cv::Rect(x * imgSize.width, y * imgSize.height,
				      imgSize.width, imgSize.height));
	  I.copyTo(im);
	}
	cv::imshow("image", Image);
	int c = cv::waitKey(5);
	if (c == 27) {
	  quit = true; break;
	} else if (c == 'n') {
	  cv::Mat_<float> m = measure.col(eidx[ei]);
	  val.copyTo(m);
	  break;
	}
      }
      if (quit) { break; }
    }
    if (quit) { break; }
    
    //write results to file
    char str[1024]; sprintf(str, "%s/%05d.txt", odir.c_str(), sample);
    std::ofstream file(str);
    if (!file.is_open()) {
      std::cout << "Failed opening file for writing!" << std::endl;
      break;
    }
    file << measure.rows << " " << measure.cols << std::endl;
    for (int i = 0; i < measure.rows; i++) {
      for (int j = 0; j < measure.cols; j++) {
	file << measure(i, j) << " ";
      }
      file << std::endl;
    }
    file.close();
    std::cout << "data written to: " << str << std::endl;
  }
  
  return 0;
}

int main(int argc, char** argv)
{
  return captureExpressions();




  
  /*
  cv::Mat I = cv::Mat::zeros(256, 256, CV_8UC3);
  cv::imshow("image", I);
  cv::waitKey(10);
  */
  
  spi_context_t* context;
  //gloop_data_spidat_t data;
  gloop_data_spidat2_t data;
  int32_t size = gloop_create_default_context(NULL);
  clock_t start_time, end_time;
  double time_span;
  int j = 0;
  if (size < 0)
    {
      printf("error sizing context.\n");
      return -1;
    }
  context = (spi_context_t*)malloc(size);
  size = gloop_create_default_context(context);
  if (size < 0)
    {
      printf("error creating spi context.\n");
      return -1;
    }
  //gloop_send_config_cmd(context, GLOOP_ODR_100HZ, GLOOP_TRGR_CONT);
  gloop_send_config_cmd2(context, GLOOP_ODR_500HZ, GLOOP_TRGR_CONT,
			 16, GLOOP_RES_001PF);

  cv::Size imgSize(128, 128);
  std::vector<Graph> G(10);
  for (int i = 0; i < 10; i++) {
    G[i].init(128, imgSize);
  } 
  int sensor = 0;
 
  start_time = clock();
  for (;;)//(size = 0; size < 1000; ++size)
    {
      int i = 0;
      usleep(5000);//Sleep(16);
      //gloop_read_data(context, &data);
      gloop_read_data2(context, &data);
      end_time = clock();
      time_span = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

      if (j < 16) {
	/*
	  printf("\n%f,", time_span); //computer time
	  printf("0x%x,", data.buf[i++]); // data type
	  printf("0x%x,", data.buf[i++]); // sequence number
	*/
	i += 2;
	for (; i < sizeof(data); i += 2) {
	  /*
	    printf("%08d,",
	    (((uint16_t) data.buf[i]) << 8) |
	    ((uint16_t) data.buf[i + 1])); // datum
	  */
	  double v =
	    ((((uint16_t) data.buf[i]) << 8) |
	     ((uint16_t) data.buf[i + 1]));
	  int k = (i - 2) / 2;
	  G[k].update(v);
	    
	}
	++j;
      }

      /*
      cv::Mat_<uchar> Image(2 * imgSize.height, 5 * imgSize.width);
      //std::cout << G[0].min_val << " " << G[0].max_val << std::endl;
      for (int k = 0; k < 10; k++) {
	int y = k / 5;
	int x = k - y * 5;
	const cv::Mat_<uchar> I = G[k].draw();
	cv::Mat im = Image(cv::Rect(x * imgSize.width, y * imgSize.height,
				    imgSize.width, imgSize.height));
	I.copyTo(im);
      }
      cv::imshow("image", Image);
      */
      const cv::Mat_<uchar> I = G[sensor].draw();
      cv::imshow("image", I);
      int key = cv::waitKey(10);
      if (key == 27) { break; }
      else if (key == 'p') {
	sensor = cv::min(sensor + 1, 9);
	std::cout << "sensor: " << sensor << std::endl;
      } else if (key == 'o') {
	sensor = cv::max(sensor - 1, 0);
	std::cout << "sensor: " << sensor << std::endl;
      }
      
      
      if (0)//(GetAsyncKeyState(VK_ESCAPE))
	{
	  break;
	}
      if (true)//(GetAsyncKeyState(VK_RETURN))
	{
	  j = 0;
	  //system("cls");
	}
    }

  gloop_free_resources(context);
  free(context);

  return 0;
}

