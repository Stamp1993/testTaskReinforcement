#pragma once
#include"NeuralNetwork.h"
#include<iterator>
static int nums = 0;

struct state {//two element pair for state allocation and its value
	VectorXd value;//type of state
	VectorXd info;
	int num;
	bool failed = false;
	double pos;
	double vel;
	double acc;
	double angle1;
	double angVel1;
	double angAcc1;

	state(int a) {
		num = nums;
		nums++;
		value = VectorXd(a);
		info = VectorXd(18);
	}

	state(int a, bool f) {
		num = nums;
		nums++;
		value = VectorXd(a);
		failed = true;
		info = VectorXd(18);
	}

	state(VectorXd vals) : value(vals) {
		num = nums;
		nums++;
		info = VectorXd(18);
	}

	state() {
		num = nums;
		nums++;
		value = VectorXd(6);
		value.Zero(6);
		info = VectorXd(18);
	}
	state(const state &oth) : num(oth.num) {
		value = oth.value;
		failed = oth.failed;
		info = oth.info;
	}

	virtual VectorXd norm() {
		return value;
	}


};

struct action {
	state* fromP;
	state* toP;
	state from;
	state to;
	double reward;

	action(state *f, state *t, double r) : fromP(f), toP(t), reward(r) {
		from = *fromP;
		to = *toP;
	}

	action(const action &oth) :fromP(oth.fromP), toP(oth.toP), reward(oth.reward), from(oth.from), to(oth.to) {

	}
};

class environment {
protected:
	int size;
	double lastReward;
	VectorXd observation;
	map<int, double> rewards;
	vector<company> companies;
public:
	state current;
	vector<action> actions;
	environment(int sz) {
		size = sz;
	}

	int getSize() {
		return size;
	}

	environment() {}

	double reward(int act) {

		//	cout << "action " << act << endl;
		action* acti = &actions[act];
		action a = *acti;
		state* curr = &a.to;
		current = *curr;
		//	cout << current.value.transpose() << endl;
		return (*this).getReward(a);
	}

	virtual double getReward(action act) {
		cout << "I'm in base!" << endl;
		lastReward = act.reward;
		return act.reward;
	}

	virtual double getReward(int day) {
		cout << "I'm in base!" << endl;
		lastReward = rewards[day-1];
		return rewards[day];
	}

	VectorXd observe() {
		VectorXd res;
		observation = VectorXd(0);
		for (auto cmp : companies) {

			VectorXd val = cmp.sendObs();
			res = VectorXd(observation.size() + val.size());
			res << observation, val;
			observation = res;
			
		}
		
		return observation;
	}

	VectorXd history() {
		VectorXd res;
		VectorXd hist = VectorXd(0);
		for (auto cmp : companies) {

			VectorXd val = cmp.oldState.value;
			res = VectorXd(hist.size() + val.size());
			res << hist, val;
			hist = res;

		}

		return hist;
	}
	
	double reward() {

		return lastReward;
	}

	virtual bool isFinal(state &st) {
		state* s = &st;
		state sta = *s;
		return sta.failed;
	}

	

	bool won(state st) {
		return 0;
	}

	void makeFinal() {}

};

class company { // company struct

	struct daily_state : public state { // encodng daily state 
		
		map<int, double> prodPrices;// products and real prices
		int day;
		VectorXd outValues;//encoded prices and products
		double avgPr;
		double reward;
		daily_state(map<int, double> &ps, int d, double ap) : state(2*prodPrices.size()), prodPrices(ps), day(d), avgPr(ap){//comstr
			makeValues(avgPr);
		}

		daily_state(): state(2){//default constr
			prodPrices = map<int, double>();
			day = 0;
		}

		daily_state(const daily_state &oth) : state(oth), prodPrices(oth.prodPrices), day(oth.day) {//copy constr
			
		}

		void makeValues(int avgPr) {//encode real prices to vectorXd
			int i = 0;
			for (auto pp: prodPrices) {
				value[2 * i] = pp.first / 1000;
				value[2 * i + 1] = pp.second / avgPr;
				i++;
			}
		}

		daily_state decode(VectorXd action) {
			
			map<int, double> nextPrices;
			int ac = action.size();
			map<int, double>::iterator it = prodPrices.begin();
			for (int i = 0; i < ac; i++) {
				
				if (action[2*i] > 0.5) {//if profitable - add
					int product = it->first;
					double price = value[2 * i + 1] * avgPr;
					nextPrices[product] = price;
					
				}
				(it++);
			}
			return daily_state(nextPrices, day + 1, avgPr);
		}
	};
public:
	daily_state currentState, oldState;//states
	neuralNetwork actionValues;//critic
	neuralNetwork policy;//actor

	company(double lr, map<int, double> ps, double d, double avgPr) { // constr
		currentState = daily_state(ps, d, avgPr);
		actionValues = neuralNetwork(60, lr, tang);
		actionValues.addLayer(60);
		actionValues.addLayer(1);

		policy = neuralNetwork(60, lr/100, logist);
		policy.addLayer(60);
		policy.addLayer(60);
		
	}

	VectorXd observe(environment env) {//get data from environment
		VectorXd inp = env.observe();	
		return inp;
	}

	VectorXd sendObs() {//send data to env
		return currentState.value;
	}

	MatrixXd neuralInput(VectorXd obs) {//transform data to view feedable to NN
		//get 30 most valuable
		VectorXd result = VectorXd::Zero(60);
		if (result.size() >= obs.size()) {
			for (int i = 0; i < obs.size(); i++) {
				result[i] = obs[i];
			}
		}
		else {
			VectorXd abses = obs.cwiseAbs();
			int spSize = ceil(obs.size() / result.size());
			int addr = 0;
			for (int i = 0; i < obs.size(); i += spSize) {
				VectorXd temp = abses.segment(spSize, i);
				VectorXd::Index ind;
				double maxVal = temp.maxCoeff(&ind);
				bool isNum = ind % 2 == 0;
				if (isNum) {
					result[addr] = obs[i + ind];
					result[addr + 1] = obs[i + ind + 1];
				}
				else {
					result[addr] = obs[i + (ind - 1)];
					result[addr + 1] = obs[i + ind];
				}
			}
		}

		return result.transpose();
	}

	VectorXd neuralOutput(VectorXd act, int cols) { //transform data to be readable from NN
		VectorXd result = VectorXd::Ones(cols);
		//TODO : transform 1xn vector to 1xm lossless
		return result.transpose();
	}



	void trainCritic(environment env, double reward) { //train critic on real data
		VectorXd obs = env.observe();
		VectorXd ni = neuralInput(obs);//transorm to NN
		VectorXd out = VectorXd::Ones(1, 1)*reward;
		VectorXd no = neuralOutput(out, ni.cols());
		actionValues.batchBackpropagation(ni, no, 0.000001);//train
	}

	VectorXd predictCritic(environment env) {
		VectorXd ni = neuralInput(env.observe());//transorm to NN
		VectorXd predict = actionValues.run(ni);//run
		return predict; //1x1 -> sum = predict

	}
	/* if Q-learning
	VectorXd trainActor(environment env, VectorXd action) { //train actor on real data
		
		VectorXd ni = neuralInput();//transorm to NN

		action = currentState.value - action;//get prediction
		action = action*env.getReward(currentState.day);//get output
		VectorXd no = neuralOutput(action, actionValues.(*output).size());
		actionValues.batchBackpropagation(ni, no, 0.0001);//can be extended to batch by Vector -> Matrix
	}*/

	//for actor-critic
	VectorXd trainActor(environment env, VectorXd action) { //train actor on real data

		VectorXd ni = neuralInput(env.observe());//transorm to NN
		VectorXd no = VectorXd::Ones(1)*currentState.reward;
		actionValues.batchBackpropagation(ni, no, 0.0001);//can be extended to batch by Vector -> Matrix
	}


	VectorXd chooseAction(environment env) {//by actor and train actor
		VectorXd inp = observe(env);//get observation

		VectorXd ni = neuralInput(inp);

		VectorXd no = policy.run(ni);//get predict from NN
		VectorXd action = neuralOutput(no);//get encoded action from NN output
		daily_state nextState = currentState.decode(action);
		oldState = currentState;
		currentState = nextState;
	}

	VectorXd trainActor(environment env) {
		VectorXd old = env.history();
		VectorXd next = env.observe();
		VectorXd was = actionValues.run(old);
		VectorXd now = actionValues.run(next);
		VectorXd action = comp(old, next);
		VectorXd ni = neuralInput(was);//input
		double Rew = (now - was).sum();
		action = action*Rew;
		VectorXd no = neuralOutput(action);
		policy.batchBackpropagation(ni, no, 0.0001);
	}

	VectorXd comp(VectorXd old, VectorXd next) {
		if (old.size() == next.size()) {
			return old - next;
		}
		else {
			if (old.size() > next.size()) {
				int diff = old.size() - next.size();
				VectorXd zer = VectorXd::Zero(diff);
				VectorXd res(old.size() + diff);
				res << next, diff;
				return old - res;
			}
			else{
				int diff = next.size() - old.size();
				VectorXd zer = VectorXd::Zero(diff);
				VectorXd res(next.size() + diff);
				res << old, diff;
				return next - res;
			}
		}
	}
	
};

