#pragma once
#include<Eigen/Dense>
#include<random>
#include<math.h>
#include<iostream>
#include<assert.h>
#include<vector>
#define princluded 0
using namespace std;
using namespace Eigen;

static default_random_engine generator;



	static double noise() {
		normal_distribution<double> distribution(0.0, 3.0);
		double trash1 = distribution(generator);//first time is the same - some mistake;
		double result = distribution(generator);
		return result;
	}

	static double noise(double in) {
		normal_distribution<double> distribution(in, 1.0);
		double trash1 = distribution(generator);//first time is the same - some mistake;
		double result = distribution(generator);
		return result;
	}

	static MatrixXd randomPR(int length, VectorXd in) {
		int width = in.size();
		MatrixXd res(length, width);
		res.col(0) = (in);
		for (int i = 1; i < length; i++) {
			VectorXd add = VectorXd::Random(width);

			res.col(i) = (add);
		}
		return res;
	}

	static MatrixXd classicPR(int length, VectorXd in) {//create pseudoSet
		int width = in.size();
		MatrixXd res(width, length);
		res.col(0) = (in);
		for (int i = 1; i < length; i++) {
			VectorXd add(width);
			for (int j = 0; j < width; j++) {
				add[j] = ((rand() % 2));
			}
			res.col(i) = (add);
		}
		return res;
	}

	static MatrixXd gaussianPR(int length, VectorXd in) {
		int width = in.size();
		MatrixXd res(length, width);
		res.col(0) = (in);
		for (int i = 1; i < length; i++) {
			VectorXd add;
			for (int j = 0; j < width; j++) {
				add[j] = (noise(in[i]));
			}
			res.col(i) = (add);
		}
		return res;
	}
