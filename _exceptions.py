class NodeProbeError(Exception) :
	def __init__(self, message, stat) :
		super().__init__(message)
		self.message = message
		self.stat = stat

	def __str__(self) :
		return f"[{self.stat}] {self.message}"
