BEGIN  {last = -10}
/findfont 9/  {
        last=NR
        $3 = 11.3
    }
    {
        if (NR == last+2) {
            $1 = $1+1
            $2 = $2-2
        }
        print
    }