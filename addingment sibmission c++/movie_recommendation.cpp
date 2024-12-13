#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

typedef vector<vector<int>> Matrix;

tuple<Matrix, vector<string>, vector<string>> loadRatings(const string &filename) {
    Matrix ratings;
    vector<string> users;
    vector<string> movies;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        exit(1);
    }

    string line;
    getline(file, line); // First line contains movie names
    stringstream ss(line);
    string item;

    getline(ss, item, ','); // Skip the first column (user IDs)
    while (getline(ss, item, ',')) {
        movies.push_back(item);
    }

    while (getline(file, line)) {
        stringstream ss(line);
        vector<int> row;
        getline(ss, item, ',');
        users.push_back(item); // First column contains user IDs
        while (getline(ss, item, ',')) {
            row.push_back(stoi(item));
        }
        ratings.push_back(row);
    }

    file.close();
    return {ratings, users, movies};
}

double calculateSimilarity(const vector<int> &userA, const vector<int> &userB) {
    int size = userA.size();
    double sumA = 0, sumB = 0, sumAB = 0;
    double sumASq = 0, sumBSq = 0;

    for (int i = 0; i < size; ++i) {
        if (userA[i] > 0 && userB[i] > 0) {
            sumA += userA[i];
            sumB += userB[i];
            sumAB += userA[i] * userB[i];
            sumASq += userA[i] * userA[i];
            sumBSq += userB[i] * userB[i];
        }
    }

    double denominator = sqrt(sumASq) * sqrt(sumBSq);
    return (denominator == 0) ? 0 : sumAB / denominator;
}

vector<double> predictRatings(const Matrix &ratings, int targetUser, const vector<double> &similarities) {
    int numMovies = ratings[0].size();
    vector<double> predictedRatings(numMovies, 0.0);
    vector<double> weightSums(numMovies, 0.0);

    for (size_t i = 0; i < ratings.size(); ++i) {
        if (i == targetUser) continue;

        double similarity = similarities[i];
        for (int j = 0; j < numMovies; ++j) {
            if (ratings[i][j] > 0) {
                predictedRatings[j] += similarity * ratings[i][j];
                weightSums[j] += abs(similarity);
            }
        }
    }

    for (int j = 0; j < numMovies; ++j) {
        if (weightSums[j] > 0) {
            predictedRatings[j] /= weightSums[j];
        }
    }

    return predictedRatings;
}

vector<pair<int, double>> recommendMovies(const vector<double> &predictedRatings, const vector<int> &userRatings, int topN) {
    vector<pair<int, double>> recommendations;

    for (int i = 0; i < predictedRatings.size(); ++i) {
        if (userRatings[i] == 0) {
            recommendations.emplace_back(i, predictedRatings[i]);
        }
    }

    sort(recommendations.begin(), recommendations.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        return b.second < a.second; // Descending order
    });

    if (recommendations.size() > topN) {
        recommendations.resize(topN);
    }

    return recommendations;
}

int main() {
    string filename = "ratings.csv";
    auto [ratings, users, movies] = loadRatings(filename);

    int targetUser = 0; // Example: Recommend for the first user

    vector<double> similarities;
    for (size_t i = 0; i < ratings.size(); ++i) {
        similarities.push_back(calculateSimilarity(ratings[targetUser], ratings[i]));
    }

    vector<double> predictedRatings = predictRatings(ratings, targetUser, similarities);
    int topN = 5; // Number of top recommendations to output

    auto recommendations = recommendMovies(predictedRatings, ratings[targetUser], topN);

    cout << "Top " << topN << " movie recommendations for user " << users[targetUser] << ":\n";
    for (const auto &[movieIndex, predictedRating] : recommendations) {
        cout << movies[movieIndex] << " (Predicted Rating: " << predictedRating << ")\n";
    }

    return 0;
}
