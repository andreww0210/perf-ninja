#include "solution.hpp"
#include <array>
#include <iostream>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

// Task: lookup all the values from l2 in l1.
// For every found value, find the sum of its digits.
// Return the sum of all digits in every found number.
// Both lists have no duplicates and elements placed in *random* order.
// Do NOT sort any of the lists. Do NOT store elements in a hash_map/sets.

// Hint: Traversing a linked list is a long data dependency chain:
//       to get the node N+1 you need to retrieve the node N first.
//       Think how you can execute multiple dependency chains in parallel.
// unsigned solution(List *l1, List *l2) {
//   unsigned retVal = 0;

//   List *head2 = l2;
//   // O(N^2) algorithm:
//   while (l1) {
//     unsigned v = l1->value;
//     l2 = head2;
//     while (l2) {
//       if (l2->value == v) {
//         retVal += getSumOfDigits(v);
//         break;
//       }
//       l2 = l2->next;
//     }
//     l1 = l1->next;
//   }

//   return retVal;
// }

// unsigned solution(List *l1, List *l2) {
//   unsigned retVal = 0;

//   int lenght2 = 0;
//   List *tmp = l2;
//   while(tmp) {
//     tmp=tmp->next;
//     lenght2++;
//   }

//   List *head2 = l2;
//   // O(N^2) algorithm:
//   while (l1) {
//     unsigned v = l1->value;
//     l2 = head2;

//     int rem = lenght2 % 4;
//     int count = lenght2 / 4;

//     bool found = false;
//     while (rem--) {
//       if (l2->value == v) {
//         retVal += getSumOfDigits(v);
//         found = true;
//         break;
//       }
//       l2 = l2->next;
//     }

//     if (!found && count) {
//       List *l20 = l2;
//       List *l21 = l2->next;//   while (l1) {
//     unsigned v = l1->value;
//     l2 = head2;
//     while (l2) {
//       if (l2->value == v) {
//         retVal += getSumOfDigits(v);
//         break;
//       }
//       l2 = l2->next;
//     }
//     l1 = l1->next;
//   }

//   return retVal;
//       List *l22 = l2->next->next;
//       List *l23 = l2->next->next->next;

//       while (l20 && l21 && l22 && l23) {
//         if (l20->value == v || l21->value == v || l22->value == v ||
//             l23->value == v) {
//           retVal += getSumOfDigits(v);
//           break;
//         }
//         count--;
//         if (!count)
//           break;
//         l20 = l20->next->next->next->next;
//         l21 = l21->next->next->next->next;
//         l22 = l22->next->next->next->next;
//         l23 = l23->next->next->next->next;
//       }
//     }

//     l1 = l1->next;
//   }

//   return retVal;
// }

template<int M>
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;

  int lenght1 = 0;
  List *tmp = l1;
  while(tmp) {
    tmp=tmp->next;
    lenght1++;
  }

  int rem = lenght1 % M;
  int count = lenght1 / M;
  
  List *head2 = l2;

  while (rem--) {
    unsigned v = l1->value;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }

  while (count--) {
    std::array<unsigned, M> vals;
    // O(N^2) algorithm:
    for (int i = 0; i < M; i++) {
      vals[i] = l1->value;
      l1 = l1->next;
    }
    l2 = head2;
    int found = 0;
    while (l2) {
      for (int i = 0; i < M; i++) {
        if (l2->value == vals[i]) {
          retVal += getSumOfDigits(l2->value);
          if (++found == M)
            break;
        }
      }
      l2 = l2->next;
    }
  }

  return retVal;
}

unsigned solution(List *l1, List *l2) {
  return solution<4>(l1, l2);
}
