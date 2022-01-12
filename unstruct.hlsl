float main(float4 pos : POSITION) {
  if (pos.x < 1)
    return pos.x;

  for (int i = 0; i < pos.y; i++) {
    if (i > pos.z)
      break;
    else if (i > 1000)
      return i;
  }

  return 0;
}