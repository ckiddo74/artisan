class NodeTable:
   class Item:
      def __init__(self, entry):
         self.__dict__ = entry
     
   def __init__(self, table):
      self.nodes = table["nodes"]
      self.keys = table["keys"]
      self.n = len(self.nodes)

      #if len(self.keys) == 1:
      #    self.get_item = self.__get_item_simple__
      #    self.key = self.keys[0]
      #else:    
      self.get_item = self.__get_item_complex__
      
   def __len__(self):
      return self.n

   '''   
   def __get_item_simple__(self, i):  
      if i < self.n:
         return self.nodes[i][self.key]
      else:
         raise IndexError 
   '''
   def __get_item_complex__(self, i):   
      if i < self.n:
         return NodeTable.Item(self.nodes[i])
      else:
         raise IndexError 
    
   def __getitem__(self, i):
      return self.get_item(i)
                
   def __iter__(self):
      self.i = 0
      return self
   
   def __next__(self):      
      if (self.i < self.n):
          item = self.get_item(self.i)
          self.i += 1                 
          return item
      else:  
          self.i = None
          raise StopIteration
   
   def __bool__(self):
      return self.n > 0   
    
   def __repr__ (self):
      return str(self.nodes)    


