import mill._, scalalib._

// If you are using VSCode to write Scala, please uncomment the following line to enable Bloop plugin:
// import $ivy.`com.lihaoyi::mill-contrib-bloop:`

object versions {
  val scala          = "2.13.10"
  val chisel         = "3.6.0"
  val chiseltest     = "0.6.0"
}

/**
 * All chisel modules should extends this trait where the chisel dependency is included.
 */
trait BaseProject extends ScalaModule {
  def scalaVersion = versions.scala

  override def ivyDeps = Agg(
    ivy"edu.berkeley.cs::chisel3:${versions.chisel}",
  )

  override def scalacOptions = Seq(
    "-language:reflectiveCalls",
    "-deprecation",
    "-feature",
    "-Xcheckinit",
  )

  override def scalacPluginIvyDeps = Agg(
    ivy"edu.berkeley.cs:::chisel3-plugin:${versions.chisel}",
    ivy"edu.berkeley.cs::chiseltest:${versions.chiseltest}",
  )

}


object hiteCPU extends BaseProject {
  def mainClass = Some("hiteCPU.Toplevel")
}

object test extends BaseProject {
  def mainClass = Some("test.Toplevel")
}
