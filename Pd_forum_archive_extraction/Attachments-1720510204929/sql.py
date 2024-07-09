import pyext, sqlite

class pysql(pyext._class):
    
    _inlets = 1
    _outlets = 1
    
    def __init__(self):
        self.db = sqlite.connect("/tmp/pypd.db")
        self.cursor = self.db.cursor()
    
    def select_1(self, pattern):
        """ select * from soundfile where description like "pattern" """
        
        # build sql with inlet-pattern:
        sql = """ SELECT * FROM soundfile where description like '%s' """ % pattern
        
        # execute query:
        self.cursor.execute(sql)
        
        # slurp everything:
        data = self.cursor.fetchall()
        
        # print each row:
        for result in data:
            self._outlet(1, result)
