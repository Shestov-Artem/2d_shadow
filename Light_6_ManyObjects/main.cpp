#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <tuple>  //��� ����������� �������� � �����������

#include <vector>
#include <algorithm>

using namespace std;

int obj_count = 0;  //������� �������� � �����

SDL_Renderer* renderer = nullptr;

SDL_Texture* LoadImage(std::string file) {
	SDL_Surface* loadedImage = nullptr;
	SDL_Texture* texture = nullptr;
	loadedImage = IMG_Load(file.c_str());
	if (loadedImage != nullptr) {
		texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	else
		std::cout << SDL_GetError() << std::endl;
	return texture;
}

//������ ����
void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
	int x = radius;
	int y = 0;
	int decisionOver2 = 1 - x; // The decision variable

	while (y <= x) {
		// Draw the circle in all octants
		SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
		SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
		SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
		SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
		SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
		SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
		SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
		SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);

		y++;

		if (decisionOver2 <= 0) {
			decisionOver2 += 2 * y + 1; // Mid-point is inside or on the perimeter of the circle
		}
		else {
			x--;
			decisionOver2 += 2 * y - 2 * x + 1; // Mid-point is outside the perimeter of the circle
		}
	}
}

class ObjectWithShadow { 

private:
	//���������-��� �������� ���������� �� ����� � �� ����������
	struct Points_Pos_and_Dist {
		double dist;
		SDL_Rect dest;
	};

	//���������� ������� �� �����������
	void bubbleSort2(Points_Pos_and_Dist arr[], int n) {
		for (int i = 0; i < n - 1; ++i) {
			for (int j = 0; j < n - i - 1; ++j) {
				if (arr[j].dist > arr[j + 1].dist) {
					// ����� ����������
					Points_Pos_and_Dist temp = arr[j];
					arr[j] = arr[j + 1];
					arr[j + 1] = temp;
				}
			}
		}
	}

	// ������� ��� ���������� �����, ������� �� �������� �������� �� �������, �������� ����� �������
	SDL_Rect findThirdPoint(SDL_Rect p1, SDL_Rect p2, double distance) {
		// ��������� ������ �����������
		double dx = p2.x - p1.x;
		double dy = p2.y - p1.y;

		// ��������� ����� �������
		double length = std::sqrt(dx * dx + dy * dy);

		// ����������� ������ �����������
		double unit_dx = dx / length;
		double unit_dy = dy / length;

		// ��������� ���������� ������� �����
		SDL_Rect p3;
		p3.x = p2.x + distance * unit_dx;
		p3.y = p2.y + distance * unit_dy;

		return p3;
	}

public:
	SDL_Rect dest;

	//�����������
	ObjectWithShadow(SDL_Texture* texture, int x, int y) {
		dest.x = x;
		dest.y = y;
		SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
		dest.w /= 6;
		dest.h /= 6;

		obj_count++;
	}

	//������� ����������� ���������� �� ��������� ����� �� �������
	int Find_min_dist(SDL_Rect dest1) {
		int min_dist;

		int distans[4];
		//������� ���������� �� ���� ������� ����� �������
		distans[0] = sqrt((dest.x - dest1.x) * (dest.x - dest1.x) + (dest.y - dest1.y) * (dest.y - dest1.y));
		distans[1] = sqrt((dest.x + dest.w - dest1.x) * (dest.x + dest.w - dest1.x) + (dest.y - dest1.y) * (dest.y - dest1.y));
		distans[2] = sqrt((dest.x + dest.w - dest1.x) * (dest.x + dest.w - dest1.x) + (dest.y + dest.h - dest1.y) * (dest.y + dest.h - dest1.y));
		distans[3] = sqrt((dest.x - dest1.x) * (dest.x - dest1.x) + (dest.y + dest.h - dest1.y) * (dest.y + dest.h - dest1.y));

		//������� ����������� ����������
		min_dist = distans[0];
		for (int i = 0; i < 4; i++) {
			if (distans[i] < min_dist) min_dist = distans[i];
		}

		return min_dist;
	}

	//�������� ���������� ������ ����������� ����� � ������� 
	void GetPosPointObj(SDL_Rect dest1, SDL_Rect dest, SDL_Rect& obj_pos_1, SDL_Rect& obj_pos_2) {

		int x = dest1.x;
		int y = dest1.y;
		int x1 = dest.x;
		int y1 = dest.y;
		int x2 = dest.x + dest.w;
		int y2 = dest.y + dest.h;

		//������� ������ �� ����������, �������� ���������� �� ����� � �� ����������
		Points_Pos_and_Dist points[4];

		//������� ���������� �� ��������� ����� �� ������� ����� �������
		points[0].dist = sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
		points[1].dist = sqrt((x - x2) * (x - x2) + (y - y1) * (y - y1));
		points[2].dist = sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
		points[3].dist = sqrt((x - x1) * (x - x1) + (y - y2) * (y - y2));

		//������� ���������� ������� ����� �������
		points[0].dest.x = x1;
		points[0].dest.y = y1;
		points[1].dest.x = x2;
		points[1].dest.y = dest.y;
		points[2].dest.x = dest.x + dest.w;
		points[2].dest.y = dest.y + dest.h;
		points[3].dest.x = dest.x;
		points[3].dest.y = dest.y + dest.h;

		//�������� �� ����������� ���������� �� �����
		bubbleSort2(points, 4);

		//������� ���������� ����� � �������� ������������ ������� ���� �����
		if ((x > x1 && x < x2) || (y > y1 && y < y2)) { //���� �������� ����� "�� ������� �� ������� �������"
			//����� ����� ����� ����� �����������

			obj_pos_1 = points[0].dest;
			obj_pos_2 = points[1].dest;
		}
		else {
			//����� ����� ����� �� ����������� � �� ������������  

			obj_pos_1 = points[1].dest;
			obj_pos_2 = points[2].dest;
		}
	}

	//�������� ���������� ������ ����������� �������� ����� � ����������� ��������� 
	void GetCirclePointsPos2(int& x_1, int& y_1, int& x_2, int& y_2, SDL_Rect dest1, SDL_Rect obj_pos_1, SDL_Rect obj_pos_2, double r) {
		//����� ����������� �������� ����� � ����������
		SDL_Rect obj_pos_3;
		SDL_Rect obj_pos_4;

		int distans1 = sqrt((obj_pos_1.x - dest1.x) * (obj_pos_1.x - dest1.x) + (obj_pos_1.y - dest1.y) * (obj_pos_1.y - dest1.y));
		int distans2 = sqrt((obj_pos_2.x - dest1.x) * (obj_pos_2.x - dest1.x) + (obj_pos_2.y - dest1.y) * (obj_pos_2.y - dest1.y));

		obj_pos_3 = findThirdPoint(dest1, obj_pos_1, r - distans1);
		obj_pos_4 = findThirdPoint(dest1, obj_pos_2, r - distans2);

		x_1 = obj_pos_3.x;
		y_1 = obj_pos_3.y;
		x_2 = obj_pos_4.x;
		y_2 = obj_pos_4.y;
	}

	//������� ����� �� ������� ����� �������� 4-� ��������
	SDL_Point OptimizationDrawShadow2(SDL_Rect dest2, SDL_Rect dest3, SDL_Rect dest5, double r, int x_1, int y_1, int x_2, int y_2, int i) {

		SDL_Point points2;
		if (sqrt((dest2.x - dest3.x) * (dest2.x - dest3.x) + (dest2.y - dest3.y) * (dest2.y - dest3.y)) < r &&
			sqrt((dest2.x - dest5.x) * (dest2.x - dest5.x) + (dest2.y - dest5.y) * (dest2.y - dest5.y)) < r) {
			if (i == 0) points2 = { dest3.x, dest3.y }; // ������� �����
			if (i == 1) points2 = { dest5.x, dest5.y }; // ������� ������
			if (i == 2) points2 = { x_2, y_2 }; // ������ ������
			if (i == 3) points2 = { x_1, y_1 };
		}
		else if (sqrt((dest2.x - dest3.x) * (dest2.x - dest3.x) + (dest2.y - dest3.y) * (dest2.y - dest3.y)) < r) {
			if (i == 0) points2 = { dest3.x, dest3.y }; // ������� �����
			if (i == 1) points2 = { dest5.x, dest5.y }; // ������� ������
			if (i == 2) points2 = { dest5.x, dest5.y }; // ������ ������
			if (i == 3) points2 = { x_1, y_1 };    // ������ �����
		}
		else if (sqrt((dest2.x - dest5.x) * (dest2.x - dest5.x) + (dest2.y - dest5.y) * (dest2.y - dest5.y)) < r) {
			if (i == 0) points2 = { dest3.x, dest3.y }; // ������� �����
			if (i == 1) points2 = { dest5.x, dest5.y }; // ������� ������
			if (i == 2) points2 = { x_2, y_2 }; // ������ ������
			if (i == 3) points2 = { dest3.x, dest3.y };    // ������ �����
		}
		else {
			if (i == 0) points2 = { dest3.x, dest3.y }; // ������� �����
			if (i == 1) points2 = { dest5.x, dest5.y }; // ������� ������
			if (i == 2) points2 = { dest5.x, dest5.y }; // ������ ������
			if (i == 3) points2 = { dest3.x, dest3.y };
		}
		return points2;
	}

	//������������ �������������� (���������������)
	void fillConvexPolygon(SDL_Renderer* renderer, SDL_Point points[], int count) {
		// ������� ������� ��������������
		int minX = points[0].x, maxX = points[0].x;
		int minY = points[0].y, maxY = points[0].y;

		for (int i = 1; i < count; ++i) {
			if (points[i].x < minX) minX = points[i].x;
			if (points[i].x > maxX) maxX = points[i].x;
			if (points[i].y < minY) minY = points[i].y;
			if (points[i].y > maxY) maxY = points[i].y;
		}

		// ������������� ���� ��� �������
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // ������ ����

		//������������
		//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // ������������� ����� ����������
		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // ������ ���� � �����-�������

		// ������ ��� �������� �����������
		std::vector<std::pair<int, int>> intersections;

		// ������������ ������ ������ �� minY �� maxY
		for (int y = minY; y <= maxY; ++y) {
			intersections.clear();

			// ������� ����������� � ������� ��������������
			for (int i = 0; i < count; ++i) {
				int j = (i + 1) % count; // ��������� �������
				if ((points[i].y <= y && points[j].y > y) || (points[i].y > y && points[j].y <= y)) {
					// ��������� x-���������� �����������
					int x = points[i].x + (y - points[i].y) * (points[j].x - points[i].x) / (points[j].y - points[i].y);
					intersections.emplace_back(x, y);
				}
			}

			// ��������� ����������� �� x
			std::sort(intersections.begin(), intersections.end());

			// ��������� ������� ����� ������ �����������
			for (size_t i = 0; i < intersections.size(); i += 2) {
				if (i + 1 < intersections.size()) {
					int xStart = intersections[i].first;
					int xEnd = intersections[i + 1].first;

					// ������ ����� ����� xStart � xEnd �� ������� ������ y
					SDL_RenderDrawLine(renderer, xStart, y, xEnd, y);
				}
			}
		}
	}

	//����������� ������ ������ ����������, ������������ ������� (���������������)
	void fillCircleSegment2(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Point point1, SDL_Point point2) {

		// ������������� ���� ��� �������
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // ����� ����

		//������������
		//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // ������������� ����� ����������
		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200); // ������ ���� � �����-�������

		// ��������� ���� � ��������
		double angle1 = atan2(point1.y - centerY, point1.x - centerX);
		double angle2 = atan2(point2.y - centerY, point2.x - centerX);

		// ��������, ��� angle1 < angle2
		if (angle1 > angle2) {
			std::swap(angle1, angle2);
		}

		// ������� ��� ����� ������ ����������
		for (int y = centerY - radius; y <= centerY + radius; y++) {
			int x_min = centerX;   //����������� �� �-�������� ��������, ��������������� ���� �������� 
			int x_max = centerX;   //������������ �� �-�������� ��������, ��������������� ���� �������� 
			bool one_x_min = true; //����� ����� ������ ��������
			int Y = centerY;       //�������� �� y-��������, ��������������� ���� �������� 
			for (int x = centerX - radius; x <= centerX + radius; x++) {
				if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radius * radius) {
					// ������� ��� ����� ������������ ������ � �����

					double angle = atan2(y - centerY, x - centerX);   //���� ����� ������ (x,y) � ������� ����������

					//����������� ������ ����� ���� �� ������ ������� �� ������ �������� �� ���� ���������� (����� ������ angle2 = angle2, � ��� angle1 = angle1 + 360)
					if (angle2 - angle1 > 3.1416) {
						//������������ �����
						if (angle >= angle2) {
							//������������ ������
							int D = (point2.x - point1.x) * (y - point1.y) - (point2.y - point1.y) * (x - point1.x);
							double angle3 = atan2(point1.y - centerY, point1.x - centerX);
							double angle4 = atan2(point2.y - centerY, point2.x - centerX);
							if (D > 0) {
								if (angle3 < angle4) {
									//������� ������ � ��������� ����, � ����� ������ �������� y
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
							else if (D <= 0) {
								if (angle3 > angle4) {
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
						}
						//������������ �����
						//if (angle <= angle1) {
						else{
							//������������ ������
							int D = (point2.x - point1.x) * (y - point1.y) - (point2.y - point1.y) * (x - point1.x);
							double angle3 = atan2(point1.y - centerY, point1.x - centerX);
							double angle4 = atan2(point2.y - centerY, point2.x - centerX);
							if (D > 0) {
								if (angle3 < angle4) {
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
							else if (D <= 0) {
								if (angle3 > angle4) {
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
						}
					}
					else {
						//������������ �����
						if (angle >= angle1 && angle <= angle2) {
							//������������ ������
							int D = (point2.x - point1.x) * (y - point1.y) - (point2.y - point1.y) * (x - point1.x);
							double angle3 = atan2(point1.y - centerY, point1.x - centerX);
							double angle4 = atan2(point2.y - centerY, point2.x - centerX);
							if (D > 0) {
								if (angle3 > angle4) {
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
							else if (D <= 0) {
								if (angle3 < angle4) {
									if (one_x_min) {
										x_min = x;
										Y = y;
										one_x_min = false;
									}
									x_max = x;
								}
							}
						}
					}
				}
			}
			SDL_RenderDrawLine(renderer, x_min, Y, x_max, Y);
		}
	}
};

//���������� �� ��������
void bubbleSort(ObjectWithShadow arr[], int dist[], int n) {
	for (int i = 0; i < n - 1; ++i) {
		for (int j = 0; j < n - i - 1; ++j) {
			if (dist[j] < dist[j + 1]) {
				// ����� ����������

				ObjectWithShadow temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;

				int temp2 = dist[j];
				dist[j] = dist[j + 1];
				dist[j + 1] = temp2;
			}
		}
	}
}

//������ ���� ���� ��������
void DrawShadows(ObjectWithShadow objects[], SDL_Rect dest1, SDL_Texture* square) {

	double r = 200; //������ ������ ����� (���� �������� �����)

	//��������� ������� �� �������� ���������� �� ���, � ����� ������� � ������ �������� ������������ ������� ��� ����, � ����� ��� ������
	int dist_objects[4];
	//������ ����������� ����������
	for (int i = 0; i < obj_count; i++)
		dist_objects[i] = objects[i].Find_min_dist(dest1);
	//�����������
	bubbleSort(objects, dist_objects, obj_count);

	//���� ������ (���� �� ��������) ��������� � ���� �������� �����, �� �������� �������� ���� � ��� ������
	for (int i = 0; i < obj_count; i++) {

		//������������ ���������� �� ��������� ����� �� ������� ����� �������
		int distans1 = sqrt((objects[i].dest.x - dest1.x) * (objects[i].dest.x - dest1.x) + (objects[i].dest.y - dest1.y) * (objects[i].dest.y - dest1.y));
		int distans2 = sqrt((objects[i].dest.x + objects[i].dest.w - dest1.x) * (objects[i].dest.x + objects[i].dest.w - dest1.x) + (objects[i].dest.y - dest1.y) * (objects[i].dest.y - dest1.y));
		int distans3 = sqrt((objects[i].dest.x + objects[i].dest.w - dest1.x) * (objects[i].dest.x + objects[i].dest.w - dest1.x) + (objects[i].dest.y + objects[i].dest.h - dest1.y) * (objects[i].dest.y + objects[i].dest.h - dest1.y));
		int distans4 = sqrt((objects[i].dest.x - dest1.x) * (objects[i].dest.x - dest1.x) + (objects[i].dest.y + objects[i].dest.h - dest1.y) * (objects[i].dest.y + objects[i].dest.h - dest1.y));
		if (distans1 <= r || distans2 <= r || distans3 <= r || distans4 <= r) {

			//�������� ���������� ������ ����������� ����� � ������� 
			SDL_Rect obj_pos_1;
			SDL_Rect obj_pos_2;
			objects[i].GetPosPointObj(dest1, objects[i].dest, obj_pos_1, obj_pos_2);

			//����� ����������� �������� ����� � ����������
			int x_1;
			int y_1;
			int x_2;
			int y_2;

			objects[i].GetCirclePointsPos2(x_1, y_1, x_2, y_2, dest1, obj_pos_1, obj_pos_2, r);

			//������� ����� 4-� ���������
			SDL_Point points[4];
			//������ ����������� (���� ���� ��� ��������� �� ������ ��)
			for (int k = 0; k < 4; k++)
				points[k] = objects[i].OptimizationDrawShadow2(dest1, obj_pos_1, obj_pos_2, r, x_1, y_1, x_2, y_2, k);

			//����� ������ 4-� �������� �� 4-� ������
			objects[i].fillConvexPolygon(renderer, points, 4);
			//����������� ������� ����������, ������������ ����� � ������
			objects[i].fillCircleSegment2(renderer, dest1.x, dest1.y, r, points[2], points[3]);

			//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			//drawCircle(renderer, dest1.x, dest1.y, r);
			//SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			//drawCircle(renderer, points[3].x, points[3].y, 20);
		}
		//������ ������
		SDL_RenderCopy(renderer, square, NULL, &objects[i].dest);
	}
}

void ApplyingGeneralShadow(ObjectWithShadow objects[], SDL_Rect dest1, SDL_Texture* square, SDL_Rect dest_shadow, SDL_Texture* shadow) {
	// �������� �������� ��� ����������
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 700, 700);
	// ��������� �������� ��� ������� ������
	SDL_SetRenderTarget(renderer, texture);
	// ������� ��������
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // ����� ����
	SDL_RenderClear(renderer);

	DrawShadows(objects, dest1, square);
	//������ ����� ����
	dest_shadow.x = dest1.x - 750 - 1;
	dest_shadow.y = dest1.y - 750;
	SDL_RenderCopy(renderer, shadow, NULL, &dest_shadow);

	// ������� � ��������� �������
	SDL_SetRenderTarget(renderer, NULL);
	// ��������� ������ ���������� ��� ��������
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MUL);   //SDL_BLENDMODE_MUL: color multiply dstRGB = (srcRGB * dstRGB) + (dstRGB * (1 - srcA)), dstA = dstA
	// ������ ����� �������� ������������ �������� ��� ���������
	SDL_SetTextureAlphaMod(texture, 220); // ��������� �����-������
	// ��������� ������������ �������� �� ������
	SDL_Rect destRect = { 0, 0, 700, 700 };
	SDL_RenderCopy(renderer, texture, NULL, &destRect);

	SDL_DestroyTexture(texture);
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "RUS");

	//�������� ������������
	int speed = 5;

	SDL_Window* window = SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 700, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//�������� �����
	SDL_Texture* light = LoadImage("red_square.png");
	SDL_Rect dest1;
	SDL_QueryTexture(light, NULL, NULL, &dest1.w, &dest1.h);
	dest1.w /= 20;
	dest1.h /= 20;
	dest1.x = 100;
	dest1.y = 100;

	//�������� �������, ������������������ � ������
	SDL_Texture* square = LoadImage("square.png");

	//�������, ����������������� � ������
	ObjectWithShadow obj_1 = ObjectWithShadow(light, 500, 300);
	ObjectWithShadow obj_2 = ObjectWithShadow(light, 200, 300);
	ObjectWithShadow obj_3 = ObjectWithShadow(light, 350, 300);

	SDL_Texture* back = LoadImage("background.png");

	SDL_Texture* shadow = LoadImage("shadow_pr.png");
	SDL_Rect dest_shadow;
	SDL_QueryTexture(shadow, NULL, NULL, &dest_shadow.w, &dest_shadow.h);
	dest_shadow.w /= 1;
	dest_shadow.h /= 1;
	dest_shadow.x = dest1.x - 750;
	dest_shadow.y = dest1.x - 750;

	const Uint8* keystates = SDL_GetKeyboardState(NULL);

	int close = 0;
	while (!close) {// animation loop

		//������������
		if (keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]) {
			dest1.y -= speed;
		}
		else if (keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT]) {
			dest1.x -= speed;
		}
		else if (keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN]) {
			dest1.y += speed;
		}
		else if (keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT]) {
			dest1.x += speed;
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {// ���������� ���������
			switch (event.type) {
			case SDL_QUIT:
				// ���������� ������� ��������
				close = 1;
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // ����� ����
		SDL_RenderClear(renderer);

		//������ � ���������, �� ������� ������������� ����
		ObjectWithShadow objects[3] = { obj_1, obj_2, obj_3 };
		
		//������ ���
		SDL_RenderCopy(renderer, back, NULL, NULL); 
		for (int i = 0; i < obj_count; i++) {
			SDL_RenderCopy(renderer, light, NULL, &objects[i].dest);
		}
		//������ ���������� ��������� ����� � ������� � ������ ��������
		SDL_Rect dest2;
		dest2.x = dest1.x - dest1.w / 2;
		dest2.y = dest1.y - dest1.h / 2;
		dest2.h = dest1.h;
		dest2.w = dest1.w;
		//������ ������ ���������� ����
		SDL_RenderCopy(renderer, light, NULL, &dest2);

		//��������� �������� ���� �� ��� ����
		ApplyingGeneralShadow(objects, dest1, square, dest_shadow, shadow);

		SDL_RenderPresent(renderer);

		SDL_Delay(1000 / 60);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}