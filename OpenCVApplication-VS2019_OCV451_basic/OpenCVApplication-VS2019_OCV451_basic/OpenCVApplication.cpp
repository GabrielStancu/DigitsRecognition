// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"

#define TRAINING_FILES_COUNT 45
#define DIGITS_SET_SIZE 11 //{0-9, '/'} 

std::vector<int> createVertProjection(Mat src)
{
	std::vector<int> projection(src.cols, 0);

	for (int j = 0; j < src.cols; j++)
	{
		for (int i = 0; i < src.rows; i++)
		{
			if (src.at<uchar>(i, j) == 255)
			{
				projection.at(j)++;
			}
		}
	}

	return projection;
}

std::vector<int> createHorProjection(Mat src)
{
	std::vector<int> projection(src.rows, 0);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 255)
			{
				projection.at(i)++;
			}
		}
	}

	return projection;
}

Mat blackAndWhiteColoredImage(Mat src)
{
	Mat dest = Mat(src.rows, src.cols, CV_8UC1);
	int threshold = 200;

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			Vec3b p = src.at<Vec3b>(i, j);
			bool background = true;

			if (p[0] > threshold || p[1] > threshold || p[2] > threshold)
			{
				background = false;
			}


			dest.at<uchar>(i, j) = background ? 0 : 255;
		}
	}

	return dest;
}

Mat applyDilation(Mat src)
{
	Mat cpy = src.clone();

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 0)
			{
				if (i > 0)
				{
					cpy.at<uchar>(i - 1, j) = 0;
				}
				if (i < src.rows - 1)
				{
					cpy.at<uchar>(i + 1, j) = 0;
				}
				if (j > 0)
				{
					cpy.at<uchar>(i, j - 1) = 0;
				}
				if (j < src.cols - 1)
				{
					cpy.at<uchar>(i, j + 1) = 0;
				}

				cpy.at<uchar>(i, j) = 0;
			}
		}
	}

	return cpy;
}

bool allNeighborsObject(Mat src, int row, int col)
{
	int whiteCode = 255;

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (row > 0 && row < src.rows - 1 && col > 0 && col < src.cols - 1 && !(i == 0 && j == 0))
			{
				if (src.at<uchar>(row + i, col + j) != whiteCode)
				{
					return false;
				}
			}
		}
	}

	return true;
}

Mat applyErosion(Mat src)
{
	Mat cpy = src.clone();

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 255)
			{
				if (!allNeighborsObject(src, i, j))
				{
					cpy.at<uchar>(i, j) = 0;
				}
			}
		}
	}

	return cpy;
}

Mat opening(Mat src)
{
	Mat dst = src.clone();
	dst = applyErosion(dst);
	dst = applyDilation(dst);

	return dst;
}

std::vector<Mat> sortCroppedImages(std::vector<Mat> croppedImages, std::vector<int> startX, std::vector<int> startY, std::vector<int>& groupSeparators,
								   int rowDeviation, int colDeviation)
{
	for (int i = 0; i < croppedImages.size() - 1; i++)
	{
		int minPos = i;
		for (int j = i + 1; j < croppedImages.size(); j++)
		{
			if (abs(startY.at(j) - startY.at(minPos)) < rowDeviation && startX.at(j) < startX.at(minPos))
			{
				minPos = j;
			}
		}

		if (minPos != i)
		{
			Mat aux = croppedImages.at(minPos);
			croppedImages.at(minPos) = croppedImages.at(i);
			croppedImages.at(i) = aux;

			int auxX = startX.at(minPos);
			startX.at(minPos) = startX.at(i);
			startX.at(i) = auxX;

			int auxY = startY.at(minPos);
			startY.at(minPos) = startY.at(i);
			startY.at(i) = auxY;
		}
	}

	for (int i = 0; i < croppedImages.size() - 1; i++)
	{
		if (abs(startX.at(i + 1) - startX.at(i)) > colDeviation || abs(startY.at(i + 1) - startY.at(i) > rowDeviation))
		{
			groupSeparators.push_back(i);
		}
	}
	groupSeparators.push_back(croppedImages.size() - 1);

	return croppedImages;
}

int computeObjectPoints(Mat src, int startRow, int endRow, int startCol, int endCol)
{
	int count = 0;

	for (int i = startRow; i <= endRow; i++)
	{
		for (int j = startCol; j < endCol; j++)
		{
			if (src.at<uchar>(i, j) == 255)
			{
				count++;
			}
		}
	}

	return count;
}

std::vector<Mat> cropDigits(Mat src, int label, Mat labels, std::vector<int>& groupSeparators)
{
	std::vector<int> startX(label + 1, src.cols);
	std::vector<int> startY(label + 1, src.cols);
	std::vector<int> endX(label + 1, -1);
	std::vector<int> endY(label + 1, -1);
	std::vector<Mat> croppedDigits;
	std::vector<int> croppedX;
	std::vector<int> croppedY;
	int reqObjectPoints = src.cols * src.rows / 4000;
	int reqHeight = src.rows / 24;

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (labels.at<int>(i, j) > 0)
			{
				int crtLabel = labels.at<int>(i, j);

				if (i < startY.at(crtLabel))
				{
					startY.at(crtLabel) = i;
				}
				else if (i > endY.at(crtLabel))
				{
					endY.at(crtLabel) = i;
				}

				if (j < startX.at(crtLabel))
				{
					startX.at(crtLabel) = j;
				}
				else if (j > endX.at(crtLabel))
				{
					endX.at(crtLabel) = j;
				}
			}
		}
	}
	
	for (int n = 0; n < label + 1; n++)
	{
		int sX = startX.at(n);
		int sY = startY.at(n);
		int eX = endX.at(n);
		int eY = endY.at(n);

		int objectPoints = computeObjectPoints(src, sY, eY, sX, eX);

		if (objectPoints > reqObjectPoints && eY - sY > reqHeight)
		{
			Mat crtDigit(eY - sY + 1, eX - sX + 1, CV_8UC1);

			for (int i = sY; i <= eY; i++)
			{
				for (int j = sX; j <= eX; j++)
				{
					crtDigit.at<uchar>(i - sY, j - sX) = src.at<uchar>(i, j);
				}
			}

			croppedDigits.push_back(crtDigit);
			croppedX.push_back(sX);
			croppedY.push_back(sY);
		}
	}

	return sortCroppedImages(croppedDigits, croppedX, croppedY, groupSeparators, src.rows / 65, src.cols / 10);
}

std::vector<Mat> labeling(Mat src, std::vector<int>& groupSeparators)
{
	int label = 0;
	Mat labels = Mat(src.rows, src.cols, CV_32SC1, Scalar(0));
	std::queue<Point> Q;

	int di[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j) == 255 && labels.at<int>(i, j) == 0)
			{
				label++;
				labels.at<int>(i, j) = label;
				Q.push(Point(j, i));

				while (!Q.empty())
				{
					Point p = Q.front();
					Q.pop();

					for (int k = 0; k < 8; k++)
					{
						int ni = p.y + di[k];
						int nj = p.x + dj[k];

						if (ni > 0 && ni < src.rows && nj > 0 && nj < src.cols)
						{
							if (src.at<uchar>(ni, nj) == 255 && labels.at<int>(ni, nj) == 0)
							{
								labels.at<int>(ni, nj) = label;
								Q.push(Point(nj, ni));
							}
						}
					}
				}
			}
		}
	}

	return cropDigits(src, label, labels, groupSeparators);
}

int computeEuclidianDistance(std::vector<int> src, std::vector<int> comp)
{
	int distance = 0;
	int n = comp.size();
	int min = (n < src.size()) ? n : src.size();
	int max = (n >= src.size()) ? n : src.size();

	for (int i = 0; i < min; i++)
	{
		int dist = src.at(i) - comp[i];
		distance += dist * dist;
	}

	if (min == n)
	{
		for (int i = min; i < src.size(); i++)
		{
			distance += src.at(i) * src.at(i);
		}
	}
	else
	{
		for (int i = min; i < n; i++)
		{
			distance += comp[i] * comp[i];
		}
	}

	return distance;
}

//Mat computeCovarianceMatrix()
//{
//
//}

//int computeMahalanobisDistance(std::vector<int> src, std::vector<int> comp, std::vector<Mat> codes)
//{
//	Mat d1(src.size(), 1, CV_32SC1, Scalar(0));
//	Mat d2(1, src.size(), CV_32SC1, Scalar(0));
//	Mat cov(src.size(), src.size(), CV_32SC1, Scalar(0));
//
//	for (int i = 0; i < src.size(); i++)
//	{
//		int diff = src.at(i) - comp.at(i);
//		d1.at<int>(i, 0) = diff;
//		d2.at<int>(0, i) = diff;
//	}
//
//	Mat res = d1 * cov * d2;
//	return res.at<int>(0, 0);
//}

char matchDigit(std::vector<int> vSrc, std::vector<int> hSrc, std::vector<Mat> verticalCodes, std::vector<Mat> horizontalCodes)
{
	int minEuclideanDistance = INT_MAX;
	std::vector<int> minDistances;
	std::vector<int> digitPos;
	int knn = 7;
	int crtMinPos = 0;
	int minPos = 0;

	for (int i = 0; i < verticalCodes.size(); i++)
	{
		for (int j = 0; j < verticalCodes.at(i).cols; j++)
		{
			Mat crtVertRow = verticalCodes.at(i).row(j);
			Mat crtHorRow = horizontalCodes.at(i).row(j);

			int euclideanDistance = computeEuclidianDistance(vSrc, crtVertRow);
			euclideanDistance += computeEuclidianDistance(hSrc, crtHorRow);

			if (crtMinPos != knn)
			{
				minDistances.push_back(euclideanDistance);
				digitPos.push_back(i);
				crtMinPos++;
			}
			else
			{
				int maxPos = 0;
				for (int l = 1; l < minDistances.size(); l++)
				{
					if (minDistances.at(l) > minDistances.at(maxPos))
					{
						maxPos = l;
					}
				}

				minDistances.at(maxPos) = euclideanDistance;
				digitPos.at(maxPos) = i;
			}
		}
	}

	std::vector<int> appCount(DIGITS_SET_SIZE, 0);
	for (int i = 0; i < digitPos.size(); i++)
	{
		int crtDigit = digitPos.at(i);
		appCount.at(crtDigit)++;
	}

	int maxPos = 0;
	for (int i = 1; i < appCount.size(); i++)
	{
		if (appCount.at(i) > appCount.at(maxPos))
		{
			maxPos = i;
		}
	}
	minPos = maxPos;

	if (minPos < 10)
	{
		char number[2];
		sprintf(number, "%d", minPos);
		return number[0];
	}
	else if (minPos == 10)
	{
		return '/';
	}
}

void recognizeDigits(std::vector<Mat> vCodes, std::vector<Mat> hCodes)
{
	char fname[MAX_PATH];

	if (openFileDlg(fname))
	{
		Mat src = imread(fname, IMREAD_COLOR);
		Mat bwImage, bwOpened;
		std::vector<Mat> labeledImages;
		std::vector<int> groupSeparators;
		int crtGroup = 0;

		bwImage = blackAndWhiteColoredImage(src);
		bwOpened = opening(bwImage);
		labeledImages = labeling(bwOpened, groupSeparators);

		for (int i = 0; i < labeledImages.size(); i++)
		{
			std::vector<int> verticalProjection = createVertProjection(labeledImages.at(i));
			std::vector<int> horizontalProjection = createHorProjection(labeledImages.at(i));
			if (i == labeledImages.size() - 1 && 
				groupSeparators.at(groupSeparators.size() - 1) - groupSeparators.at(groupSeparators.size() - 2) > 2)
			{
				printf(".");
			}
			char matchedDigit = matchDigit(verticalProjection, horizontalProjection, vCodes, hCodes);
			printf("%c", matchedDigit);

			if (i == groupSeparators.at(crtGroup) || i == labeledImages.size() - 1)
			{
				crtGroup++;
				printf("\n");
			}
		}
	}
}

void trainSystem(std::vector<Mat>& verticalProjections, std::vector<Mat>& horizontalProjections)
{
	char fNames[TRAINING_FILES_COUNT][MAX_PATH];
	char expectedValues[TRAINING_FILES_COUNT][18] = {
		"129/711016713378",
		"131/79998620354", 
		"130/80998320367",
		"129/721037711368",
		"123/79948020382",
		"129/73948412375",
		"124/83968615394",
		"121/77877213365",
		"122/751028315388",
		"121/76868516368",
		"121/831028715381",
		"130/761016911394",
		"124/81877721392",
		"132/72958614397",
		"130/82104791837",
		"130/81958614385",
		"123/83937712394",
		"123/831027815372",
		"121/771037518352",
		"123/74916621395",
		"128/83897621372",
		"133/78898511372",
		"123/72100671235",
		"128/73886613365",
		"133/84977020362",
		"123/831007418359",
		"123/76878820362",
		"120/701047916384",
		"134/7495671140",
		"122/76857617364",
		"125/70101801240",
		"128/72927614357",
		"134/71977820351",
		"132/70947121394",
		"124/74908217393",
		"129/72998410357",
		"134/7492701339",
		"126/84977821353",
		"121/72868216384",
		"133/831026717362",
		"120/74897911393",
		"133/8085851536",
		"129/72897911375",
		"121/711037013371",
		"126/71998014356"
	};
	std::vector<int> appCount(DIGITS_SET_SIZE, 0);
	std::vector<std::vector<int>> vCodes;
	std::vector<std::vector<int>> hCodes;

	for (int i = 0; i < DIGITS_SET_SIZE; i++)
	{
		verticalProjections.push_back(Mat(250, 250, CV_32SC1, Scalar(0)));
		horizontalProjections.push_back(Mat(250, 250, CV_32SC1, Scalar(0)));

		std::vector<int> emptyVector(50, 0);
		vCodes.push_back(emptyVector);
		hCodes.push_back(emptyVector);
	}

	for (int i = 0; i < TRAINING_FILES_COUNT; i++)
	{
		sprintf(fNames[i], "Images/TrainingSet/%d.jpg", (i + 1));

		Mat src = imread(fNames[i], IMREAD_COLOR);
		Mat bwImage, bwOpened;
		std::vector<Mat> labeledImages;
		std::vector<int> groupSeparators; 

		bwImage = blackAndWhiteColoredImage(src);
		bwOpened = opening(bwImage);
		labeledImages = labeling(bwOpened, groupSeparators);

		for (int j = 0; j < labeledImages.size(); j++)
		{
			std::vector<int> verticalProjection = createVertProjection(labeledImages.at(j));
			std::vector<int> horizontalProjection = createHorProjection(labeledImages.at(j));
			char expectedDigit = expectedValues[i][j];
			int digitPos;

			if (expectedDigit != '/')
			{
				digitPos = expectedDigit - '0';
			}
			else
			{
				digitPos = 10;
			}

			for (int v = 0; v < verticalProjection.size(); v++)
			{
				verticalProjections.at(digitPos).at<int>(appCount.at(digitPos), v) = verticalProjection.at(v);
			}

			for (int h = 0; h < horizontalProjection.size(); h++)
			{
				horizontalProjections.at(digitPos).at<int>(appCount.at(digitPos), h) = horizontalProjection.at(h);
			}

			appCount.at(digitPos)++;
		}

		printf("Analyzed image %d / %d\n", (i + 1), TRAINING_FILES_COUNT);
	}
}

int main()
{
	std::vector<Mat> verticalProjections, horizontalProjections;
	trainSystem(verticalProjections, horizontalProjections);
	char op = 'y';
	do
	{
		recognizeDigits(verticalProjections, horizontalProjections);
		printf("Parse another image? y - yes, other key - no\n");
		scanf("%c", &op);
		getchar();
	} while (op == 'y');
	
	return 0;
}