#include<iostream>
#include<map>
#include"Environment.h"
#include"FileWorks.h"

int dateDD(vector<string> inp) {//get date to int
	vector<string> els = split(inp[2], '-');
	int dat = 1 * stoi(els[0]) + 100 * stoi(els[1]) + stoi(els[2]);
	return dat;
}

int datePD(vector<string> inp) {//get date to int
	vector<string> els = split(inp[0], '.');
	int dat = 1 * stoi(els[2]) + 100 * stoi(els[1]) + stoi(els[0]);
	return dat;
}

int main() {
	SSvec daysData = readFile("срез_по_дням.csv", ',');
	SSvec profitsData = readFile("доходы_UC.csv", ';');

	daysData.erase(daysData.begin());//delete first row
	profitsData.erase(profitsData.begin());//delete first row
	cout << daysData[0][0] << " " << daysData[0][1] << endl;//check
	cout << profitsData[0][0] << " " << profitsData[0][1] << endl;


	map<int, double> profits;
	map<pair<int, string>, vector<string>> compData;
	for (auto prof : profitsData) {//profits map
		profits[datePD(prof)] = stod(prof[1]);
	}
	for (auto dat : daysData) {//company databy days map
		pair<int, string> addr(dateDD(dat), dat[0]);
		if (compData.find(addr) == compData.end())
			compData[addr] = vector<string>();
		compData[pair<int, string>(dateDD(dat), dat[0])].push_back(dat[1] + ';'+dat[3]);
	}
	daysData.clear();
	profitsData.clear();

	map<pair<int, string>, map<int, double>> dailyPrices;//day, comp-> producs, prices
	
	for (auto cmp : compData) {
		vector<string> dat = cmp.second;
		map<int, double> prices;
		for (auto d : dat) {

		}

	}


	environment env();

	company UC();

	system("pause");
}