Scenario Outline: User withdraws money from a bank
  Given <user> logs into bank and has balance of <n1> coins
  When <user> withdraws <n2> coins
  Then <user> has <n3> coins left
  Examples:
    | user    | n1 | n2 | n3 |
    | alice   |  5 |  1 |  4 |
    | bob     | 10 |  3 |  7 |
