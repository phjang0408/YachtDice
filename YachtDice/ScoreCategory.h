#pragma once
// 고정 순서, 배열 인덱스로 바로 사용, UI-Player-Scorer 공통분모
enum class ScoreCategory {
	Ones = 0,
	Twos,
	Threes,
	Fours,
	Fives,
	Sixes,

	Choice,			// 합
	FourOfKind,		// 포 카드
	FullHouse,		// 풀하우스
	SmallStraight,	// 스몰 스트레이트
	LargeStraight,	// 라지 스트레이트
	Yacht,			// 야추

	COUNT			// 족보 개수 - 12
};