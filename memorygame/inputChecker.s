.global inputChecker
.extern currentIndex
.extern userInput
.extern sequence

.text
inputChecker:
  push r4
  push r5
  push r6

  mov.b &currentIndex, r4      ; r4 = currentIndex
  mov   r12, r5                ; r5 = shapeId

  ; userInput[r4] = shapeId
  mov   #userInput, r6
  add   r4, r6
  mov.b r5, 0(r6)

  ; compare to sequence[r4]
  mov   #sequence, r6
  add   r4, r6
  mov.b 0(r6), r6              ; r6 = sequence[r4]

  cmp.b r5, r6                 ; shapeId == expected?
  jne   wrong

  mov #0, r12                  ; correct input
  jmp done

wrong:
  mov #1, r12                  ; incorrect input

done:
  pop r6
  pop r5
  pop r4
  ret
